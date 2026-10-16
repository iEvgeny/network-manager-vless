/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

    Compatibility declaration for the Plasma NetworkManager editor API.
    The upstream header is not installed by plasma-nm; keep this declaration in
    sync with plasma-nm 5.27 and 6.x (libs/editor/widgets/settingwidget.h).
*/
#ifndef NM_VLESS_PLASMA_SETTING_WIDGET_H
#define NM_VLESS_PLASMA_SETTING_WIDGET_H

#include "plasmanm_editor_export.h"

#include <NetworkManagerQt/Setting>
#include <QWidget>

#include <KAcceleratorManager>

class PLASMANM_EDITOR_EXPORT SettingWidget : public QWidget
{
    Q_OBJECT
public:
    class EnumPasswordStorageType
    {
    public:
        enum PasswordStorageType {
            Store = 0,
            AlwaysAsk,
            NotRequired
        };
    };

    explicit SettingWidget(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(),
                           QWidget *parent = nullptr,
                           Qt::WindowFlags flags = {});
    explicit SettingWidget(const NetworkManager::Setting::Ptr &setting,
                           const QStringList &hints,
                           QWidget *parent = nullptr,
                           Qt::WindowFlags flags = {});
    ~SettingWidget() override;

    virtual void loadConfig(const NetworkManager::Setting::Ptr &setting);
    virtual void loadSecrets(const NetworkManager::Setting::Ptr &setting);
    virtual QVariantMap setting() const = 0;
    void watchChangedSetting();
    QString type() const;

    virtual bool isValid() const
    {
        return true;
    }

protected Q_SLOTS:
    void slotWidgetChanged();

Q_SIGNALS:
    void validChanged(bool isValid);
    void settingChanged();

protected:
    QStringList m_hints;

private:
    QString m_type;
};

#endif // NM_VLESS_PLASMA_SETTING_WIDGET_H
