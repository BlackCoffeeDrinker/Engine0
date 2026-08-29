#include <catch2/catch_test_macros.hpp>

#include <Engine.hpp>
#include <Engine/Platform/ResourceManager.hpp>
#include <Engine/Platform/StreamFactory.hpp>
#include <Engine/World.hpp>

#include "Actors/ActorChest.hpp"
#include "Loaders/TilesetLoader.hpp"

#include <filesystem>
#include <fstream>
#include <functional>

using namespace e00;

namespace {
/**
 * A `TestEngine` whose `MakeActorForType` can be overridden per-test via `actorFactoryOverride`,
 * standing in for the world-file `source` types used in each test ("chest", "player", ...) that
 * aren't part of `Engine::MakeActorForType`'s built-in set.
 */
class TestEngine : public Engine {
public:
  [[nodiscard]] std::string_view Name() const noexcept override { return "Engine00_ActorTests"; }

  std::function<std::unique_ptr<Actor>(const std::string_view &, const std::map<std::string, std::string> &)> actorFactoryOverride;

protected:
  std::unique_ptr<Actor> MakeActorForType(const std::string_view &type, const std::map<std::string, std::string> &properties) override {
    if (actorFactoryOverride) {
      if (auto actor = actorFactoryOverride(type, properties)) {
        return actor;
      }
    }

    return Engine::MakeActorForType(type, properties);
  }
};

void WriteFile(const std::filesystem::path &path, std::string_view content) {
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out.write(content.data(), static_cast<std::streamsize>(content.size()));
}

// A minimal concrete `Actor` (no sprites of its own) used to stand in for the player avatar
// in this persistence test.
class TestPlayerActor : public Actor {
public:
  [[nodiscard]] type_t Type() const override { return type_id<TestPlayerActor>(); }

protected:
  [[nodiscard]] const State &GetCurrentState() const noexcept override {
    static const State kEmptyState{0, {}};
    return kEmptyState;
  }
};

/**
 * Sets up a scratch resource directory (with an empty `state/` subfolder) and points the global
 * `StreamFactory` at it so `Engine::LoadWorld` can find world `.ini` files and read/write
 * per-world actor state files, without needing any real tileset/palette/sprite assets.
 */
std::filesystem::path SetUpScratchResourceDirectory() {
  const auto dir = std::filesystem::temp_directory_path() / "engine00_actor_persistence_tests";
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir / "state");

  StreamFactory::GlobalStreamFactory().SetResourceDirectory(dir.string() + "/");

  // `World::AddTileset` eagerly checks that a loader + backing stream exist for the tileset
  // resource named in `[map] tileset = ...` (even though nothing in this test ever actually
  // dereferences/parses it), so a loader and a placeholder file are required here.
  auto &resourceManager = ResourceManager::GlobalResourceManager();
  static const auto registerTilesetLoaderOnce = resourceManager.AddLoader<impl::TilesetLoader>();
  (void) registerTilesetLoaderOnce;

  resourceManager.SetAlias(HashName("dummy_tileset"), "dummy_tileset.ini");
  WriteFile(dir / "dummy_tileset.ini", "[tileset]\n");

  return dir;
}
}// namespace

TEST_CASE("Engine::LoadWorld persists ActorChest state across an unload/reload cycle", "[engine][actor][persistence]") {
  const auto dir = SetUpScratchResourceDirectory();

  std::vector<impl::ActorChest *> createdChests;

  WriteFile(dir / "world_a.ini",
            "[map]\n"
            "width = 4\n"
            "height = 4\n"
            "tileset = 1:dummy_tileset\n"
            "\n"
            "[set]\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n"
            "\n"
            "[object:CHEST1]\n"
            "source = chest\n"
            "position = 1, 1\n");

  WriteFile(dir / "world_b.ini",
            "[map]\n"
            "width = 4\n"
            "height = 4\n"
            "tileset = 1:dummy_tileset\n"
            "\n"
            "[set]\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n");

  TestEngine engine;
  engine.actorFactoryOverride = [&createdChests](const std::string_view &type, const std::map<std::string, std::string> &) -> std::unique_ptr<Actor> {
    if (type != "chest") {
      return nullptr;
    }

    auto chest = std::make_unique<impl::ActorChest>();
    createdChests.push_back(chest.get());
    return chest;
  };

  auto &resourceManager = ResourceManager::GlobalResourceManager();
  resourceManager.SetAlias(HashName("world_a"), "world_a.ini");
  resourceManager.SetAlias(HashName("world_b"), "world_b.ini");

  REQUIRE_FALSE(engine.LoadWorld("world_a"));
  REQUIRE(createdChests.size() == 1);
  REQUIRE_FALSE(createdChests[0]->IsOpen());

  // Open the chest.
  createdChests[0]->OnInteract(*engine.CurrentWorld(), 0);
  REQUIRE(createdChests[0]->IsOpen());

  // Leave to a different world: this should snapshot the open chest's state and unload it.
  REQUIRE_FALSE(engine.LoadWorld("world_b"));

  // Coming back should reconstruct the chest (a new instance) and restore it as open.
  REQUIRE_FALSE(engine.LoadWorld("world_a"));
  REQUIRE(createdChests.size() == 2);
  REQUIRE(createdChests[1]->IsOpen());

  std::filesystem::remove_all(dir);
}

TEST_CASE("A Global actor survives loading an unrelated world", "[engine][actor][persistence]") {
  const auto dir = SetUpScratchResourceDirectory();

  std::vector<Actor *> createdPlayers;

  WriteFile(dir / "world_a.ini",
            "[map]\n"
            "width = 4\n"
            "height = 4\n"
            "tileset = 1:dummy_tileset\n"
            "\n"
            "[set]\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n"
            "\n"
            "[object:PLAYER]\n"
            "source = player\n"
            "position = 2, 2\n");

  WriteFile(dir / "world_b.ini",
            "[map]\n"
            "width = 4\n"
            "height = 4\n"
            "tileset = 1:dummy_tileset\n"
            "\n"
            "[set]\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n"
            "data = 1,1,1,1,\n"
            "\n"
            "[entry:from_a]\n"
            "position = 3, 1\n");

  TestEngine engine;
  engine.actorFactoryOverride = [&createdPlayers](const std::string_view &type, const std::map<std::string, std::string> &) -> std::unique_ptr<Actor> {
    if (type != "player") {
      return nullptr;
    }

    auto player = std::make_unique<TestPlayerActor>();
    player->SetPersistence(Actor::Persistence::Global);
    createdPlayers.push_back(player.get());
    return player;
  };

  auto &resourceManager = ResourceManager::GlobalResourceManager();
  resourceManager.SetAlias(HashName("world_a"), "world_a.ini");
  resourceManager.SetAlias(HashName("world_b"), "world_b.ini");

  REQUIRE_FALSE(engine.LoadWorld("world_a"));
  REQUIRE(createdPlayers.size() == 1);
  REQUIRE(engine.CurrentWorld()->NumActors() == 1);
  REQUIRE(createdPlayers[0]->Position() == WorldPosition{2, 2});

  // world_b doesn't define a player object at all: if the Global actor were erased like a
  // WorldLocal one, the factory would never be called again to recreate it (since it's not part
  // of world_b's actor list), so `NumActors` would drop to 0 and stay there permanently.
  REQUIRE_FALSE(engine.LoadWorld("world_b"));
  REQUIRE(createdPlayers.size() == 1);
  REQUIRE(engine.CurrentWorld()->NumActors() == 1);
  REQUIRE(createdPlayers[0]->Position() == WorldPosition{0, 0});

  // Named entry point places the Global actor at the declared spawn.
  REQUIRE_FALSE(engine.LoadWorld("world_b", "from_a"));
  REQUIRE(createdPlayers.size() == 1);
  REQUIRE(engine.CurrentWorld()->NumActors() == 1);
  REQUIRE(createdPlayers[0]->Position() == WorldPosition{3, 1});

  // Unknown entry point falls back to {0,0} rather than the previous world position.
  REQUIRE_FALSE(engine.LoadWorld("world_b", "missing_entry"));
  REQUIRE(createdPlayers[0]->Position() == WorldPosition{0, 0});

  std::filesystem::remove_all(dir);
}
