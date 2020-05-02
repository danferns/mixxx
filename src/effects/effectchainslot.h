#pragma once

#include <QDomDocument>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSignalMapper>

#include "control/controlobject.h"
#include "effects/defs.h"
#include "effects/presets/effectchainpreset.h"
#include "engine/channelhandle.h"
#include "util/class.h"
#include "util/memory.h"

class ControlPushButton;
class ControlEncoder;
class EffectChainSlot;
class EffectsManager;
class EffectProcessor;
class EngineEffectChain;

/// EffectChainSlot is the main thread representation of an effect chain which
/// adds/removes exactly one EngineEffectChain from the engine. Unlike EffectSlot,
/// EffectChainSlot does not add/remove EngineEffectChains apart from Mixxx
/// startup and shutdown. The lifetime of EngineEffectChain is coupled with
/// EffectChainSlot. Do not change this relationship; there is no use case for
/// that.
///
/// EffectChainSlot owns the ControlObjects for the routing switches that assign
/// chains to process audio inputs (decks, microphones, auxiliary inputs,
/// master mix). EffectChainSlot also owns the ControlObject for the superknob
/// which manipulates the metaknob of each effect in the chain.
///
/// The state of an EffectChainSlot can be saved to and loaded from an
/// EffectChainPreset, which can serialize/deserialize that state to/from XML.
class EffectChainSlot : public QObject {
    Q_OBJECT
  public:
    EffectChainSlot(const QString& group,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger,
            SignalProcessingStage stage = SignalProcessingStage::Postfader);
    virtual ~EffectChainSlot();

    QString group() const;

    EffectSlotPointer getEffectSlot(unsigned int slotNumber);

    void registerInputChannel(const ChannelHandleAndGroup& handle_group,
                              const double initialValue = 0.0);
    QSet<ChannelHandleAndGroup> getActiveChannels() const {
        return m_enabledInputChannels;
    }

    double getSuperParameter() const;
    void setSuperParameter(double value, bool force = false);

    EffectChainMixMode mixMode() const {
        return m_mixMode;
    }
    void setMixMode(EffectChainMixMode mixMode);

    const QString& getGroup() const {
        return m_group;
    }

    const QString& presetName() const;
    void setPresetName(const QString& name);

    // Get the human-readable description of the EffectChain
    QString description() const;
    void setDescription(const QString& description);

    static QString mixModeToString(EffectChainMixMode type) {
        switch (type) {
            case EffectChainMixMode::DrySlashWet:
                return "DRY/WET";
            case EffectChainMixMode::DryPlusWet:
                return "DRY+WET";
            default:
                return "UNKNOWN";
        }
    }
    static EffectChainMixMode mixModeFromString(const QString& typeStr) {
        if (typeStr == "DRY/WET") {
            return EffectChainMixMode::DrySlashWet;
        } else if (typeStr == "DRY+WET") {
            return EffectChainMixMode::DryPlusWet;
        } else {
            return EffectChainMixMode::NumMixModes;
        }
    }

    const QList<EffectSlotPointer>& getEffectSlots() const {
        return m_effectSlots;
    }

    virtual void loadEffectWithDefaults(
            const unsigned int iEffectSlotNumber,
            const EffectManifestPointer pManifest);

    void loadChainPreset(EffectChainPresetPointer pPreset);

  public slots:
    void slotControlClear(double value);

  signals:
    void nameChanged(const QString& name);

  protected slots:
    void sendParameterUpdate();
    void slotControlChainSuperParameter(double v, bool force = false);

  protected:
    EffectSlotPointer addEffectSlot(const QString& group);

    // Activates EffectChain processing for the provided channel.
    void enableForInputChannel(const ChannelHandleAndGroup& handle_group);
    void disableForInputChannel(const ChannelHandleAndGroup& handle_group);

    EffectsManager* m_pEffectsManager;
    EffectChainPresetManagerPointer m_pChainPresetManager;
    EffectsMessengerPointer m_pMessenger;
    ControlObject* m_pControlChainMix;
    ControlObject* m_pControlChainSuperParameter;
    QList<EffectSlotPointer> m_effectSlots;

  private slots:
    void slotControlLoadChainPreset(double value);
    void slotControlChainSelector(double value);
    void slotControlChainNextPreset(double value);
    void slotControlChainPrevPreset(double value);
    void slotChannelStatusChanged(double value, const ChannelHandleAndGroup& handle_group);

  private:
    QString debugString() const {
        return QString("EffectChainSlot(%1)").arg(m_group);
    }

    void addToEngine();
    void removeFromEngine();

    const QString m_group;

    ControlPushButton* m_pControlClear;
    ControlObject* m_pControlNumEffectSlots;
    ControlObject* m_pControlChainLoaded;
    ControlPushButton* m_pControlChainEnabled;
    ControlPushButton* m_pControlChainMixMode;
    ControlObject* m_pControlLoadPreset;
    ControlObject* m_pControlLoadedPreset;
    ControlEncoder* m_pControlChainSelector;
    ControlPushButton* m_pControlChainNextPreset;
    ControlPushButton* m_pControlChainPrevPreset;

    /**
      These COs do not affect how the effects are processed;
      they are defined here for skins and controller mappings to communicate
      with each other. They cannot be defined in skins because they must be present
      when both skins and mappings are loaded, otherwise the skin will
      create a new CO with the same ConfigKey but actually be interacting with a different
      object than the mapping.
    **/
    ControlPushButton* m_pControlChainShowFocus;
    ControlPushButton* m_pControlChainHasControllerFocus;
    ControlPushButton* m_pControlChainShowParameters;
    ControlPushButton* m_pControlChainFocusedEffect;

    QString m_presetName;
    EffectChainMixMode m_mixMode;
    SignalProcessingStage m_signalProcessingStage;
    QHash<ChannelHandleAndGroup, std::shared_ptr<ControlPushButton>> m_channelEnableButtons;
    QSet<ChannelHandleAndGroup> m_enabledInputChannels;
    EngineEffectChain* m_pEngineEffectChain;

    DISALLOW_COPY_AND_ASSIGN(EffectChainSlot);
};
