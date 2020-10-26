/**
* @file midicontroller.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Tue 7 Feb 2012
* @brief MIDI Controller base class
*
* This is a base class representing a MIDI controller.
*   It must be inherited by a class that implements it on some API.
*
*   Note that the subclass' destructor should call close() at a minimum.
*/

#ifndef MIDICONTROLLER_H
#define MIDICONTROLLER_H

#include "controllers/controller.h"
#include "controllers/midi/midicontrollerpreset.h"
#include "controllers/midi/midicontrollerpresetfilehandler.h"
#include "controllers/midi/midimessage.h"
#include "controllers/midi/midioutputhandler.h"
#include "controllers/softtakeover.h"

class MidiController : public Controller {
    Q_OBJECT
  public:
    explicit MidiController();
    ~MidiController() override;

    ControllerJSProxy* jsProxy() override;

    QString presetExtension() override;

    ControllerPresetPointer getPreset() const override {
        MidiControllerPreset* pClone = new MidiControllerPreset();
        *pClone = m_preset;
        return ControllerPresetPointer(pClone);
    }

    void visit(const MidiControllerPreset* preset) override;
    void visit(const HidControllerPreset* preset) override;

    void accept(ControllerVisitor* visitor) override {
        if (visitor) {
            visitor->visit(this);
        }
    }

    bool isMappable() const override {
        return m_preset.isMappable();
    }

    bool matchPreset(const PresetInfo& preset)  override;

  signals:
    void messageReceived(unsigned char status, unsigned char control,
                         unsigned char value);

  protected:
    virtual void sendShortMsg(unsigned char status,
            unsigned char byte1,
            unsigned char byte2) = 0;

    /// Alias for send()
    /// The length parameter is here for backwards compatibility for when scripts
    /// were required to specify it.
    inline void sendSysexMsg(QList<int> data, unsigned int length = 0) {
        Q_UNUSED(length);
        send(data);
    }

  protected slots:
    virtual void receiveShortMessage(
            unsigned char status,
            unsigned char control,
            unsigned char value,
            mixxx::Duration timestamp);
    // For receiving System Exclusive messages
    void receive(const QByteArray& data, mixxx::Duration timestamp) override;
    int close() override;

  private slots:
    bool applyPreset() override;

    void learnTemporaryInputMappings(const MidiInputMappings& mappings);
    void clearTemporaryInputMappings();
    void commitTemporaryInputMappings();

  private:
    void processInputMapping(
            const MidiInputMapping& mapping,
            unsigned char status,
            unsigned char control,
            unsigned char value,
            mixxx::Duration timestamp);
    void processInputMapping(
            const MidiInputMapping& mapping,
            const QByteArray& data,
            mixxx::Duration timestamp);

    double computeValue(MidiOptions options, double _prevmidivalue, double _newmidivalue);
    void createOutputHandlers();
    void updateAllOutputs();
    void destroyOutputHandlers();

    /// Returns a pointer to the currently loaded controller preset. For internal
    /// use only.
    ControllerPreset* preset() override {
        return &m_preset;
    }

    QHash<uint16_t, MidiInputMapping> m_temporaryInputMappings;
    QList<MidiOutputHandler*> m_outputs;
    MidiControllerPreset m_preset;
    SoftTakeoverCtrl m_st;
    QList<QPair<MidiInputMapping, unsigned char> > m_fourteen_bit_queued_mappings;

    // So it can access sendShortMsg()
    friend class MidiOutputHandler;
    friend class MidiControllerTest;
    friend class MidiControllerJSProxy;
};

class MidiControllerJSProxy : public ControllerJSProxy {
    Q_OBJECT
  public:
    MidiControllerJSProxy(MidiController* m_pController)
            : ControllerJSProxy(m_pController),
              m_pMidiController(m_pController) {
    }

    Q_INVOKABLE void sendShortMsg(unsigned char status,
            unsigned char byte1,
            unsigned char byte2) {
        m_pMidiController->sendShortMsg(status, byte1, byte2);
    }

    Q_INVOKABLE void sendSysexMsg(QList<int> data, unsigned int length = 0) {
        m_pMidiController->sendSysexMsg(data, length);
    }

  private:
    MidiController* m_pMidiController;
};

#endif
