#pragma once

#include "../../core/device/DeviceController.h"
#include "../../core/persistence/SettingsRepository.h"

#include <QList>
#include <QWidget>

class QListWidget;
class QPushButton;

class EffectsPage final : public QWidget {
    Q_OBJECT

public:
    explicit EffectsPage(
        SettingsRepository* settings,
        QWidget* parent = nullptr
    );
    void setDevice(DeviceController* device);
    static QList<EffectPreset> builtInEffects();

private:
    void rebuild();
    void playSelected();
    void createEffect();
    void duplicateSelected();
    void editSelected();
    void deleteSelected();
    void saveCustomEffects();
    [[nodiscard]] int selectedIndex() const;

    SettingsRepository* settings_;
    DeviceController* device_ = nullptr;
    QList<EffectPreset> effects_;
    int builtInCount_ = 0;
    QListWidget* list_;
    QPushButton* playButton_;
    QPushButton* stopButton_;
    QPushButton* editButton_;
    QPushButton* deleteButton_;
};

