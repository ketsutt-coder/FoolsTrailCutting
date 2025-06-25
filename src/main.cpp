#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/CCMotionStreak.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <unordered_map>

using namespace geode::prelude;

// i assume some geode dev is gonna review this to know if this should be approved or not
// apologies for the shit code in advance LOL
// re-learning c++ while learning geode and gd modding is kinda hard (and not ideal ik)
// if anything's like VERY fucked up lmk and i'll get on it :)

static std::unordered_map<CCMotionStreak*, bool> streakStates;
static bool trailExternallyTriggered = false;
static bool shouldCallOriginal = true;

static double cutFreq = Mod::get()->getSettingValue<double>("cutting-freq");

$on_mod(Loaded) {
    listenForSettingChanges("cutting-freq", [](double value) {
        cutFreq = value;
    });
}

class $modify(EpicTrailMod, CCMotionStreak) {
    struct Fields {
        float elapsedTime = cutFreq / 2;
        bool isCutting = false;
    };

    virtual void update(float delta) {
        CCMotionStreak::update(delta);

        auto fields = m_fields.self();

        if (streakStates[this]) {
            fields->elapsedTime += delta;

            if (fields->elapsedTime >= cutFreq) {
                fields->elapsedTime -= cutFreq;

                if (fields->isCutting) {
                    this->stopStroke();
                } else {
                    this->resumeStroke();
                }

                fields->isCutting = !fields->isCutting;
            }
        }
    }
};

class $modify(LePlayerObjete, PlayerObject) {
    void activateStreak() {

        if (m_regularTrail) {
            auto streak = static_cast<CCMotionStreak*>(m_regularTrail);
            if (streak) {
                streakStates[streak] = true;
            }
        }

        if (shouldCallOriginal) {
            PlayerObject::activateStreak();
            shouldCallOriginal = false;
        } else {
            if (m_isDart) {
                PlayerObject::activateStreak();
            }
        }
    }

    void hitGround(GameObject* p0, bool p1) {
        PlayerObject::hitGround(p0, p1);

        if (!m_isShip && !m_isSwing && !m_isDart && !m_isBird) {
            if (m_regularTrail) {
                auto streak = static_cast<CCMotionStreak*>(m_regularTrail);
                // idk why i did this but let's see how it behaves
                streak->stopStroke();
                if (streak) {
                    streakStates[streak] = false;
                    shouldCallOriginal = true;
                }
            }
        }
    }

    // fix visibility bug
    // teehee
    void setVisible(bool p0) {
        PlayerObject::setVisible(p0);

        auto streak = static_cast<CCMotionStreak*>(m_regularTrail);
        streak->setVisible(p0);
    }

    void update(float delta) {
        PlayerObject::update(delta);

        if (m_isShip || m_isSwing || m_isDart || m_isBird) {
            if (m_regularTrail) {
                auto streak = static_cast<CCMotionStreak*>(m_regularTrail);
                if (streak) {
                    streakStates[streak] = true;
                    shouldCallOriginal = false;
                }
            }
        }

        if (trailExternallyTriggered) {
            trailExternallyTriggered = false;
        }
    }
};
