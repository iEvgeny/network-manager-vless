/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "plasma-nm/vpnuiplugin.h"

class VlessUiPlugin final : public VpnUiPlugin
{
    Q_OBJECT
public:
    explicit VlessUiPlugin(QObject *parent = nullptr, const QVariantList &arguments = QVariantList());
    ~VlessUiPlugin() override;

    SettingWidget *widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent) override;
    SettingWidget *askUser(const NetworkManager::VpnSetting::Ptr &setting,
                           const QStringList &hints,
                           QWidget *parent) override;
    QString suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const override;
    QStringList supportedFileExtensions() const override;
    ImportResult importConnectionSettings(const QString &fileName) override;
};
