#include "Config.h"
#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"

static uint32 dungeonSpellId    = 0;
static uint32 raidSpellId       = 0;

class player_dungeon_buff final : public PlayerScript
{
public:
    player_dungeon_buff() : PlayerScript("player_dungeon_buff") { }

    void OnPlayerMapChanged(Player* player) override
    {
        if (dungeonSpellId != 0)
        {
            if (player->GetMap()->IsNonRaidDungeon())
                player->CastSpell(player, dungeonSpellId, true);
            else
                player->RemoveAurasDueToSpell(dungeonSpellId);
        }

        if (raidSpellId != 0)
        {
            if (player->GetMap()->IsRaid())
                player->CastSpell(player, raidSpellId, true);
            else
                player->RemoveAurasDueToSpell(raidSpellId);
        }
    }
};

class world_dungeon_buff final : public WorldScript
{
public:
    world_dungeon_buff() : WorldScript("world_dungeon_buff") { }

    void OnAfterConfigLoad(bool reload) override
    {
        if (!reload)
            return;

        uint32 oldDungeonSpellId    = dungeonSpellId;
        uint32 oldRaidSpellId       = raidSpellId;

        InitializeSpells();

        if (dungeonSpellId == oldDungeonSpellId && raidSpellId == oldRaidSpellId)
            return;

        // Reload case - remove old and apply new auras if needed
        std::shared_lock<std::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
        HashMapHolder<Player>::MapType const& players = ObjectAccessor::GetPlayers();
        for (HashMapHolder<Player>::MapType::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            Player* player = itr->second;

            if (oldDungeonSpellId != 0)
                player->RemoveAurasDueToSpell(oldDungeonSpellId);

            if (oldRaidSpellId)
                player->RemoveAurasDueToSpell(oldRaidSpellId);

            Map const* map = player->FindMap();
            if (!map)
                continue;

            if (dungeonSpellId != 0 && map->IsNonRaidDungeon())
                player->CastSpell(player, dungeonSpellId, true);

            if (raidSpellId != 0 && map->IsRaid())
                player->CastSpell(player, raidSpellId, true);
        }
    }

    void OnBeforeWorldInitialized() override
    {
        InitializeSpells();
    }

private:
    void InitializeSpells() const
    {
        dungeonSpellId  = sConfigMgr->GetOption<uint32>("DungeonBuff.Spell.Dungeon", 0);
        raidSpellId     = sConfigMgr->GetOption<uint32>("DungeonBuff.Spell.Raid", 0);
        ValidateSpells();
    }

    void ValidateSpells() const
    {
        if (dungeonSpellId != 0 && !sSpellMgr->GetSpellInfo(dungeonSpellId))
        {
            LOG_ERROR("server.loading", "DungeonBuff.Spell.Dungeon contains a non-existent spell {}. Set to 0.", dungeonSpellId);
            dungeonSpellId = 0;
        }

        if (raidSpellId != 0 && !sSpellMgr->GetSpellInfo(raidSpellId))
        {
            LOG_ERROR("server.loading", "DungeonBuff.Spell.Raid contains a non-existent spell {}. Set to 0.", raidSpellId);
            raidSpellId = 0;
        }
    }
};

void AddDungeonBuffScripts()
{
    new player_dungeon_buff();
    new world_dungeon_buff();
}
