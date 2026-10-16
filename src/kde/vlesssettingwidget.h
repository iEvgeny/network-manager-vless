/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "plasma-nm/settingwidget.h"

#include <NetworkManagerQt/VpnSetting>
#include <QProcess>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QFormLayout;

class VlessSettingWidget final : public SettingWidget
{
public:
    explicit VlessSettingWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent = nullptr);
    ~VlessSettingWidget() override;

    QVariantMap setting() const override;
    bool isValid() const override;

private:
    enum class ImportKind {
        Link,
        Subscription,
    };

    void loadVpnSetting(const NetworkManager::VpnSetting::Ptr &setting);
    void startImport();
    void runCli(const QStringList &arguments, const QByteArray &input, ImportKind kind);
    void finishCli(QProcess *process, int exitCode, QProcess::ExitStatus exitStatus, ImportKind kind);
    void updateLinkButton();
    void updateVisibility();
    void setStatus(const QString &message, bool isError = false);

    NMStringMap m_originalData;
    NMStringMap m_originalSecrets;

    QTabWidget *m_tabs = nullptr;
    QLineEdit *m_linkEdit = nullptr;
    QPushButton *m_linkButton = nullptr;
    QLabel *m_linkStatus = nullptr;
    QLineEdit *m_address = nullptr;
    QSpinBox *m_port = nullptr;
    QLineEdit *m_uuid = nullptr;
    QComboBox *m_security = nullptr;
    QLineEdit *m_sni = nullptr;
    QComboBox *m_fingerprint = nullptr;
    QLineEdit *m_alpn = nullptr;
    QComboBox *m_flow = nullptr;
    QLineEdit *m_publicKey = nullptr;
    QLineEdit *m_shortId = nullptr;
    QLineEdit *m_spiderX = nullptr;
    QComboBox *m_network = nullptr;
    QLineEdit *m_path = nullptr;
    QLineEdit *m_host = nullptr;
    QLineEdit *m_grpcService = nullptr;
    QComboBox *m_grpcMode = nullptr;
    QComboBox *m_xhttpMode = nullptr;
    QLineEdit *m_xhttpExtra = nullptr;
    QLineEdit *m_encryption = nullptr;
    QSpinBox *m_mtu = nullptr;
    QLineEdit *m_dns = nullptr;
    QLineEdit *m_dns6 = nullptr;
    QCheckBox *m_ipv6 = nullptr;
    QFormLayout *m_securityForm = nullptr;
    QFormLayout *m_transportForm = nullptr;
    QProcess *m_process = nullptr;
    QString m_uuidFlags;
    bool m_hadMtu = false;
    bool m_hadIpv6 = false;
};

class VlessAuthWidget final : public SettingWidget
{
public:
    explicit VlessAuthWidget(const NetworkManager::VpnSetting::Ptr &setting,
                             const QStringList &hints,
                             QWidget *parent = nullptr);

    QVariantMap setting() const override;
    bool isValid() const override;

private:
    QLineEdit *m_uuid = nullptr;
};
