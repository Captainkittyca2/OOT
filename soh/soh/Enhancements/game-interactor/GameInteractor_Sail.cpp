#ifdef ENABLE_REMOTE_CONTROL

#include "GameInteractor_Sail.h"
#include <libultraship/bridge.h>
#include <libultraship/libultraship.h>
#include <nlohmann/json.hpp>

// MARK: - Declarations

template <class DstType, class SrcType>
bool IsType(const SrcType* src) {
  return dynamic_cast<const DstType*>(src) != nullptr;
}

template <class TypeA>
bool TakesParameter() {
    return std::is_base_of_v<ParameterizedGameInteractionEffect, TypeA>;
}

template <class TypeA>
bool IsRemovable() {
    return std::is_base_of_v<RemovableGameInteractionEffect, TypeA>;
}

/// Map of string name to enum value and flag whether it takes in a param or not
std::unordered_map<std::string, std::tuple<GameInteractorSailEffects, bool, bool>> nameToEnum = {
    {"modify_heart_container", {
        GameInteractorSailEffects::modifyHeartContainers,
        IsRemovable<GameInteractionEffect::ModifyHeartContainers>(),
        TakesParameter<GameInteractionEffect::ModifyHeartContainers>()
    }},
    { "fill_magic", {
        GameInteractorSailEffects::fillMagic,
        IsRemovable<GameInteractionEffect::ModifyHeartContainers>(),
        TakesParameter<GameInteractionEffect::FillMagic>()
    }},
    { "empty_magic", {
        GameInteractorSailEffects::emptyMagic,
        IsRemovable<GameInteractionEffect::EmptyMagic>(),
        TakesParameter<GameInteractionEffect::EmptyMagic>()
    }},
    { "modify_rupees", {
        GameInteractorSailEffects::modifyRupees,
        IsRemovable<GameInteractionEffect::ModifyRupees>(),
        TakesParameter<GameInteractionEffect::ModifyRupees>()
    }},
    { "no_ui", {
        GameInteractorSailEffects::noUI,
        IsRemovable<GameInteractionEffect::NoUI>(),
        TakesParameter<GameInteractionEffect::NoUI>()
    }},
    { "modify_gravity", {
        GameInteractorSailEffects::modifyGravity,
        IsRemovable<GameInteractionEffect::ModifyGravity>(),
        TakesParameter<GameInteractionEffect::ModifyGravity>()
    }},
    { "modify_health", {
        GameInteractorSailEffects::modifyHealth,
        IsRemovable<GameInteractionEffect::ModifyHealth>(),
        TakesParameter<GameInteractionEffect::ModifyHealth>()
    }},
    { "set_player_health", {
        GameInteractorSailEffects::setPlayerHealth,
        IsRemovable<GameInteractionEffect::SetPlayerHealth>(),
        TakesParameter<GameInteractionEffect::SetPlayerHealth>()
    }},
    { "freeze_player", {
        GameInteractorSailEffects::freezePlayer,
        IsRemovable<GameInteractionEffect::FreezePlayer>(),
        TakesParameter<GameInteractionEffect::FreezePlayer>()
    }},
    { "burn_player", {
        GameInteractorSailEffects::burnPlayer,
        IsRemovable<GameInteractionEffect::BurnPlayer>(),
        TakesParameter<GameInteractionEffect::BurnPlayer>()
    }},
    { "electrocute_player", {
        GameInteractorSailEffects::electrocutePlayer,
        IsRemovable<GameInteractionEffect::ElectrocutePlayer>(),
        TakesParameter<GameInteractionEffect::ElectrocutePlayer>()
    }},
    { "knockback_player", {
        GameInteractorSailEffects::knockbackPlayer,
        IsRemovable<GameInteractionEffect::KnockbackPlayer>(),
        TakesParameter<GameInteractionEffect::KnockbackPlayer>()
    }},
    { "modify_link_size", {
        GameInteractorSailEffects::modifyLinkSize,
        IsRemovable<GameInteractionEffect::ModifyLinkSize>(),
        TakesParameter<GameInteractionEffect::ModifyLinkSize>()
    }},
    { "invisible_link", {
        GameInteractorSailEffects::invisibleLink,
        IsRemovable<GameInteractionEffect::InvisibleLink>(),
        TakesParameter<GameInteractionEffect::InvisibleLink>()
    }},
    { "pacifist_mode", {
        GameInteractorSailEffects::pacifistMode,
        IsRemovable<GameInteractionEffect::PacifistMode>(),
        TakesParameter<GameInteractionEffect::PacifistMode>()
    }},
    { "disable_z_targeting", {
        GameInteractorSailEffects::disableZTargeting,
        IsRemovable<GameInteractionEffect::DisableZTargeting>(),
        TakesParameter<GameInteractionEffect::DisableZTargeting>()
    }},
    { "weather_rainstorm", {
        GameInteractorSailEffects::weatherRainstorm,
        IsRemovable<GameInteractionEffect::WeatherRainstorm>(),
        TakesParameter<GameInteractionEffect::WeatherRainstorm>()
    }},
    { "reverse_controls", {
        GameInteractorSailEffects::reverseControls,
        IsRemovable<GameInteractionEffect::ReverseControls>(),
        TakesParameter<GameInteractionEffect::ReverseControls>()
    }},
    { "force_equip_boots", {
        GameInteractorSailEffects::forceEquipBoots,
        IsRemovable<GameInteractionEffect::ForceEquipBoots>(),
        TakesParameter<GameInteractionEffect::ForceEquipBoots>()
    }},
    { "modify_run_speed_modifier", {
        GameInteractorSailEffects::modifyRunSpeedModifier,
        IsRemovable<GameInteractionEffect::ModifyRunSpeedModifier>(),
        TakesParameter<GameInteractionEffect::ModifyRunSpeedModifier>()
    }},
    { "one_hit_ko", {
        GameInteractorSailEffects::oneHitKO,
        IsRemovable<GameInteractionEffect::OneHitKO>(),
        TakesParameter<GameInteractionEffect::OneHitKO>()
    }},
    { "modify_defense_modifier", {
        GameInteractorSailEffects::modifyDefenseModifier,
        IsRemovable<GameInteractionEffect::ModifyDefenseModifier>(),
        TakesParameter<GameInteractionEffect::ModifyDefenseModifier>()
    }},
    { "give_or_take_shield", {
        GameInteractorSailEffects::giveOrTakeShield,
        IsRemovable<GameInteractionEffect::GiveOrTakeShield>(),
        TakesParameter<GameInteractionEffect::GiveOrTakeShield>()
    }},
};

void GameInteractorSail::Enable() {
    if (isEnabled) {
        return;
    }

    isEnabled = true;
    GameInteractor::Instance->EnableRemoteInteractor();
    GameInteractor::Instance->RegisterRemoteJsonHandler([&](nlohmann::json payload) {
        HandleRemoteJson(payload);
    });
}

void GameInteractorSail::Disable() {
    if (!isEnabled) {
        return;
    }

    isEnabled = false;
    GameInteractor::Instance->DisableRemoteInteractor();
}

void GameInteractorSail::HandleRemoteJson(nlohmann::json payload) {
    SPDLOG_INFO("[GameInteractorSail] Received payload: \n{}\n", payload.dump());
    
    if (!payload.contains("id") || !payload.contains("effect") || !payload["effect"].contains("type")) {
        return;
    }

    std::string type = payload["effect"]["type"].get<std::string>();

    if (type == "console") {
        std::string command = payload["effect"]["command"].get<std::string>();
        std::reinterpret_pointer_cast<LUS::ConsoleWindow>(LUS::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Console"))->Dispatch(command);
        GameInteractor::Instance->TransmitJsonToRemote(payload);
    } else if (type == "apply" || type == "remove") {
        GameInteractionEffectBase* giEffect = EffectFromJson(payload["effect"]["name"].get<std::string>(), payload["effect"]);
        if (giEffect) {
            if (type == "apply") {
                auto result = giEffect->Apply();
                if (result != GameInteractionEffectQueryResult::TemporarilyNotPossible) {
                    GameInteractor::Instance->TransmitJsonToRemote(payload);
                } else {
                    payload["id"] = NULL;
                    GameInteractor::Instance->TransmitJsonToRemote(payload);
                }
            } else if (IsType<RemovableGameInteractionEffect>(giEffect)) {
                dynamic_cast<RemovableGameInteractionEffect*>(giEffect)->Remove();
                GameInteractor::Instance->TransmitJsonToRemote(payload);
            }
        }
    }
}

GameInteractionEffectBase* GameInteractorSail::EffectFromJson(std::string name, nlohmann::json payload) {
    if (nameToEnum.find(name) == nameToEnum.end()) {
        return nullptr;
    }

    switch (std::get<0>(nameToEnum[name])) {
        case GameInteractorSailEffects::modifyHeartContainers: {
            auto effect = new GameInteractionEffect::ModifyHeartContainers();
            if (payload.contains("parameters")) {
                effect->parameters[0] = payload["parameters"][0].get<int32_t>();
            }
            return effect;
        }
        case GameInteractorSailEffects::fillMagic:
            return new GameInteractionEffect::FillMagic();
        case GameInteractorSailEffects::emptyMagic:
            return new GameInteractionEffect::EmptyMagic();
        case GameInteractorSailEffects::modifyRupees: {
            auto effect = new GameInteractionEffect::ModifyRupees();
            if (payload.contains("parameters")) {
                effect->parameters[0] = payload["parameters"][0].get<int32_t>();
            }
            return effect;
        }
        case GameInteractorSailEffects::noUI:
            return new GameInteractionEffect::NoUI();
        case GameInteractorSailEffects::modifyGravity: {
            auto effect = new GameInteractionEffect::ModifyGravity();
            if (payload.contains("parameters")) {
                effect->parameters[0] = payload["parameters"][0].get<int32_t>();
            }
            return effect;
        }
        case GameInteractorSailEffects::modifyHealth: {
            auto effect = new GameInteractionEffect::ModifyHealth();
            if (payload.contains("parameters")) {
                effect->parameters[0] = payload["parameters"][0].get<int32_t>();
            }
            return effect;
        }
        case GameInteractorSailEffects::setPlayerHealth: {
            auto effect = new GameInteractionEffect::SetPlayerHealth();
            if (payload.contains("parameters")) {
                effect->parameters[0] = payload["parameters"][0].get<int32_t>();
            }
            return effect;
        }
        case GameInteractorSailEffects::freezePlayer:
            return new GameInteractionEffect::FreezePlayer();
        case GameInteractorSailEffects::burnPlayer:
            return new GameInteractionEffect::BurnPlayer();
        case GameInteractorSailEffects::electrocutePlayer:
            return new GameInteractionEffect::ElectrocutePlayer();
        case GameInteractorSailEffects::knockbackPlayer: {
            auto effect = new GameInteractionEffect::KnockbackPlayer();
            if (payload.contains("parameters")) {
                effect->parameters[0] = payload["parameters"][0].get<int32_t>();
            }
            return effect;
        }
        case GameInteractorSailEffects::modifyLinkSize: {
            auto effect = new GameInteractionEffect::ModifyLinkSize();
            if (payload.contains("parameters")) {
                effect->parameters[0] = payload["parameters"][0].get<int32_t>();
            }
            return effect;
        }
        case GameInteractorSailEffects::invisibleLink:
            return new GameInteractionEffect::InvisibleLink();
        case GameInteractorSailEffects::pacifistMode:
            return new GameInteractionEffect::PacifistMode();
        case GameInteractorSailEffects::disableZTargeting:
            return new GameInteractionEffect::DisableZTargeting();
        case GameInteractorSailEffects::weatherRainstorm:
            return new GameInteractionEffect::WeatherRainstorm();
        case GameInteractorSailEffects::reverseControls:
            return new GameInteractionEffect::ReverseControls();
        case GameInteractorSailEffects::forceEquipBoots: {
            auto effect = new GameInteractionEffect::ForceEquipBoots();
            if (payload.contains("parameters")) {
                effect->parameters[0] = payload["parameters"][0].get<int32_t>();
            }
            return effect;
        }
        case GameInteractorSailEffects::modifyRunSpeedModifier: {
            auto effect = new GameInteractionEffect::ModifyRunSpeedModifier();
            if (payload.contains("parameters")) {
                effect->parameters[0] = payload["parameters"][0].get<int32_t>();
            }
            return effect;
        }
        case GameInteractorSailEffects::oneHitKO:
            return new GameInteractionEffect::OneHitKO();
        case GameInteractorSailEffects::modifyDefenseModifier: {
            auto effect = new GameInteractionEffect::ModifyDefenseModifier();
            if (payload.contains("parameters")) {
                effect->parameters[0] = payload["parameters"][0].get<int32_t>();
            }
            return effect;
        }
        case GameInteractorSailEffects::giveOrTakeShield: {
            auto effect = new GameInteractionEffect::GiveOrTakeShield();
            if (payload.contains("parameters")) {
                effect->parameters[0] = payload["parameters"][0].get<int32_t>();
            }
            return effect;
        }
        default:
            return nullptr;
    }
}

#endif
