/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "vlesssettingwidget.h"

#include "nm-vless-common.h"

#include <KLocalizedString>
#include <NetworkManagerQt/GenericTypes>

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include <array>
#include <initializer_list>
#include <utility>

namespace {
constexpr auto SecretUuid = NM_VLESS_SECRET_UUID;
constexpr int DefaultPort = NM_VLESS_DEFAULT_PORT;
constexpr int DefaultMtu = NM_VLESS_DEFAULT_MTU;

const char *const ManagedDataKeys[] = {
    NM_VLESS_KEY_ADDRESS,            NM_VLESS_KEY_PORT,               NM_VLESS_KEY_ENCRYPTION,
    NM_VLESS_KEY_FLOW,               NM_VLESS_KEY_NETWORK,             NM_VLESS_KEY_SECURITY,
    NM_VLESS_KEY_SNI,                NM_VLESS_KEY_FINGERPRINT,         NM_VLESS_KEY_ALPN,
    NM_VLESS_KEY_REALITY_PUBLIC_KEY, NM_VLESS_KEY_REALITY_SHORT_ID,    NM_VLESS_KEY_REALITY_SPIDER_X,
    NM_VLESS_KEY_PATH,               NM_VLESS_KEY_HOST,                NM_VLESS_KEY_GRPC_SERVICE_NAME,
    NM_VLESS_KEY_MODE,               NM_VLESS_KEY_XHTTP_EXTRA,        NM_VLESS_KEY_MTU,
    NM_VLESS_KEY_DNS,                NM_VLESS_KEY_DNS6,                NM_VLESS_KEY_IPV6,
    NM_VLESS_KEY_UUID_FLAGS,         nullptr,
};

void addChoice(QComboBox *combo, const QString &label, const char *value)
{
    combo->addItem(label, QString::fromLatin1(value));
}

void setComboValue(QComboBox *combo, const QString &value)
{
    int index = combo->findData(value);
    if (index < 0) {
        combo->addItem(value, value);
        index = combo->count() - 1;
    }
    combo->setCurrentIndex(index);
}

QString comboValue(const QComboBox *combo)
{
    return combo->currentData().toString();
}

void setFormRowVisible(QFormLayout *form, QWidget *field, bool visible)
{
    field->setVisible(visible);
    if (QWidget *label = form->labelForField(field)) {
        label->setVisible(visible);
    }
}

QGroupBox *formGroup(QWidget *parent, const QString &title, QFormLayout **form)
{
    auto *group = new QGroupBox(title, parent);
    auto *layout = new QFormLayout(group);
    layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    *form = layout;
    return group;
}

QString dataValue(const NMStringMap &map, const QString &key, const QString &fallback = {})
{
    return map.value(key, fallback);
}

NetworkManager::VpnSetting::Ptr toQtVpnSetting(NMSettingVpn *setting)
{
    if (!setting) {
        return {};
    }

    auto result = NetworkManager::VpnSetting::Ptr(new NetworkManager::VpnSetting);
    result->setServiceType(QString::fromUtf8(nm_setting_vpn_get_service_type(setting)));

    NMStringMap data;
    guint dataCount = 0;
    char **dataKeys = nm_setting_vpn_get_data_keys(setting, &dataCount);
    for (guint i = 0; i < dataCount; ++i) {
        const char *value = nm_setting_vpn_get_data_item(setting, dataKeys[i]);
        if (value) {
            data.insert(QString::fromUtf8(dataKeys[i]), QString::fromUtf8(value));
        }
    }
    g_strfreev(dataKeys);
    result->setData(data);

    NMStringMap secrets;
    guint secretCount = 0;
    char **secretKeys = nm_setting_vpn_get_secret_keys(setting, &secretCount);
    for (guint i = 0; i < secretCount; ++i) {
        const char *value = nm_setting_vpn_get_secret(setting, secretKeys[i]);
        if (value) {
            secrets.insert(QString::fromUtf8(secretKeys[i]), QString::fromUtf8(value));
        }
    }
    g_strfreev(secretKeys);
    result->setSecrets(secrets);
    return result;
}

QVariantMap vpnSettingMap(const NMStringMap &data, const NMStringMap &secrets)
{
    NetworkManager::VpnSetting vpn;
    vpn.setServiceType(QString::fromLatin1(NM_VLESS_SERVICE_TYPE));
    vpn.setData(data);
    vpn.setSecrets(secrets);
    return vpn.toMap();
}
} // namespace

VlessSettingWidget::VlessSettingWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
    : SettingWidget(setting, parent)
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    m_tabs = new QTabWidget(this);
    outerLayout->addWidget(m_tabs);

    // Connection tab: link import and the fields used by every VLESS profile.
    auto *connectionPage = new QWidget(m_tabs);
    auto *connectionLayout = new QVBoxLayout(connectionPage);

    auto *importGroup = new QGroupBox(i18n("Import from link"), connectionPage);
    auto *importLayout = new QVBoxLayout(importGroup);
    auto *linkRow = new QHBoxLayout;
    m_linkEdit = new QLineEdit(importGroup);
    m_linkEdit->setPlaceholderText(i18n("Paste a vless:// link or an https:// subscription URL"));
    m_linkEdit->setClearButtonEnabled(true);
    m_linkButton = new QPushButton(i18n("Fill In"), importGroup);
    m_linkButton->setEnabled(false);
    linkRow->addWidget(m_linkEdit, 1);
    linkRow->addWidget(m_linkButton);
    importLayout->addLayout(linkRow);
    m_linkStatus = new QLabel(importGroup);
    m_linkStatus->setWordWrap(true);
    m_linkStatus->hide();
    importLayout->addWidget(m_linkStatus);
    connectionLayout->addWidget(importGroup);

    QFormLayout *serverForm = nullptr;
    auto *serverGroup = formGroup(connectionPage, i18n("Server"), &serverForm);
    m_address = new QLineEdit(serverGroup);
    m_address->setPlaceholderText(i18n("vpn.example.com"));
    serverForm->addRow(i18n("Server address"), m_address);
    m_port = new QSpinBox(serverGroup);
    m_port->setRange(1, 65535);
    m_port->setValue(DefaultPort);
    serverForm->addRow(i18n("Port"), m_port);
    m_uuid = new QLineEdit(serverGroup);
    m_uuid->setEchoMode(QLineEdit::Password);
    m_uuid->setClearButtonEnabled(true);
    m_uuid->setPlaceholderText(i18n("UUID or VLESS user ID"));
    serverForm->addRow(i18n("User ID"), m_uuid);
    connectionLayout->addWidget(serverGroup);
    connectionLayout->addStretch(1);
    m_tabs->addTab(connectionPage, i18n("Connection"));

    // TLS / REALITY settings.
    auto *securityPage = new QWidget(m_tabs);
    auto *securityLayout = new QVBoxLayout(securityPage);
    auto *securityGroup = formGroup(securityPage, i18n("Security"), &m_securityForm);
    m_security = new QComboBox(securityGroup);
    addChoice(m_security, i18n("None"), "none");
    addChoice(m_security, i18n("TLS"), "tls");
    addChoice(m_security, i18n("REALITY"), "reality");
    m_securityForm->addRow(i18n("Security"), m_security);
    m_sni = new QLineEdit(securityGroup);
    m_sni->setPlaceholderText(i18n("Same as the server address"));
    m_securityForm->addRow(i18n("Server name (SNI)"), m_sni);
    m_fingerprint = new QComboBox(securityGroup);
    addChoice(m_fingerprint, i18n("Default"), "");
    for (const auto &[label, value] : std::array<std::pair<QString, const char *>, 10>{{
             {i18n("Chrome"), "chrome"}, {i18n("Firefox"), "firefox"},
             {i18n("Safari"), "safari"}, {i18n("iOS"), "ios"},
             {i18n("Android"), "android"}, {i18n("Edge"), "edge"},
             {i18n("360"), "360"}, {i18n("QQ"), "qq"},
             {i18n("Random"), "random"}, {i18n("Randomized"), "randomized"},
         }}) {
        addChoice(m_fingerprint, label, value);
    }
    m_securityForm->addRow(i18n("TLS fingerprint"), m_fingerprint);
    m_alpn = new QLineEdit(securityGroup);
    m_alpn->setPlaceholderText(QStringLiteral("h2,http/1.1"));
    m_securityForm->addRow(i18n("ALPN"), m_alpn);
    m_flow = new QComboBox(securityGroup);
    addChoice(m_flow, i18n("Default"), "");
    addChoice(m_flow, i18n("XTLS Vision"), "xtls-rprx-vision");
    m_securityForm->addRow(i18n("Flow"), m_flow);
    m_publicKey = new QLineEdit(securityGroup);
    m_securityForm->addRow(i18n("REALITY public key"), m_publicKey);
    m_shortId = new QLineEdit(securityGroup);
    m_securityForm->addRow(i18n("REALITY short ID"), m_shortId);
    m_spiderX = new QLineEdit(securityGroup);
    m_spiderX->setPlaceholderText(QStringLiteral("/"));
    m_securityForm->addRow(i18n("REALITY spider X"), m_spiderX);
    securityLayout->addWidget(securityGroup);
    securityLayout->addStretch(1);
    m_tabs->addTab(securityPage, i18n("Security"));

    // Transport-specific options.
    auto *transportPage = new QWidget(m_tabs);
    auto *transportLayout = new QVBoxLayout(transportPage);
    auto *transportGroup = formGroup(transportPage, i18n("Transport"), &m_transportForm);
    m_network = new QComboBox(transportGroup);
    addChoice(m_network, i18n("TCP"), "tcp");
    addChoice(m_network, i18n("XHTTP"), "xhttp");
    addChoice(m_network, i18n("WebSocket"), "ws");
    addChoice(m_network, i18n("HTTPUpgrade"), "httpupgrade");
    addChoice(m_network, i18n("gRPC"), "grpc");
    m_transportForm->addRow(i18n("Network"), m_network);
    m_path = new QLineEdit(transportGroup);
    m_path->setPlaceholderText(QStringLiteral("/"));
    m_transportForm->addRow(i18n("Path"), m_path);
    m_host = new QLineEdit(transportGroup);
    m_host->setPlaceholderText(i18n("Same as the server address"));
    m_transportForm->addRow(i18n("HTTP host"), m_host);
    m_grpcService = new QLineEdit(transportGroup);
    m_transportForm->addRow(i18n("gRPC service name"), m_grpcService);
    m_grpcMode = new QComboBox(transportGroup);
    addChoice(m_grpcMode, i18n("Default (gun)"), "");
    addChoice(m_grpcMode, i18n("gun"), "gun");
    addChoice(m_grpcMode, i18n("multi"), "multi");
    m_transportForm->addRow(i18n("gRPC mode"), m_grpcMode);
    m_xhttpMode = new QComboBox(transportGroup);
    addChoice(m_xhttpMode, i18n("Default (auto)"), "");
    addChoice(m_xhttpMode, i18n("auto"), "auto");
    addChoice(m_xhttpMode, i18n("packet-up"), "packet-up");
    addChoice(m_xhttpMode, i18n("stream-up"), "stream-up");
    addChoice(m_xhttpMode, i18n("stream-one"), "stream-one");
    m_transportForm->addRow(i18n("XHTTP mode"), m_xhttpMode);
    m_xhttpExtra = new QLineEdit(transportGroup);
    m_xhttpExtra->setPlaceholderText(i18n("JSON object (optional)"));
    m_transportForm->addRow(i18n("XHTTP extra settings"), m_xhttpExtra);
    transportLayout->addWidget(transportGroup);
    transportLayout->addStretch(1);
    m_tabs->addTab(transportPage, i18n("Transport"));

    // Local TUN and routing options.
    auto *advancedPage = new QWidget(m_tabs);
    auto *advancedLayout = new QVBoxLayout(advancedPage);
    QFormLayout *advancedForm = nullptr;
    auto *advancedGroup = formGroup(advancedPage, i18n("Tunnel"), &advancedForm);
    m_encryption = new QLineEdit(advancedGroup);
    m_encryption->setText(QStringLiteral("none"));
    advancedForm->addRow(i18n("Encryption"), m_encryption);
    m_mtu = new QSpinBox(advancedGroup);
    m_mtu->setRange(576, 9000);
    m_mtu->setValue(DefaultMtu);
    advancedForm->addRow(i18n("MTU"), m_mtu);
    m_dns = new QLineEdit(advancedGroup);
    m_dns->setPlaceholderText(QStringLiteral("1.1.1.1,1.0.0.1"));
    advancedForm->addRow(i18n("IPv4 DNS servers"), m_dns);
    m_dns6 = new QLineEdit(advancedGroup);
    m_dns6->setPlaceholderText(QStringLiteral("2606:4700:4700::1111,2606:4700:4700::1001"));
    advancedForm->addRow(i18n("IPv6 DNS servers"), m_dns6);
    m_ipv6 = new QCheckBox(i18n("Route IPv6 through the tunnel"), advancedGroup);
    m_ipv6->setChecked(true);
    advancedForm->addRow(QString(), m_ipv6);
    advancedLayout->addWidget(advancedGroup);
    advancedLayout->addStretch(1);
    m_tabs->addTab(advancedPage, i18n("Advanced"));

    loadVpnSetting(setting);

    connect(m_linkEdit, &QLineEdit::textChanged, this, [this] { updateLinkButton(); });
    connect(m_linkButton, &QPushButton::clicked, this, [this] { startImport(); });
    const auto watchTextField = [this](QLineEdit *field) {
        connect(field, &QLineEdit::textChanged, this, [this] {
            Q_EMIT settingChanged();
            slotWidgetChanged();
        });
    };
    for (QLineEdit *field : {m_address, m_uuid, m_sni, m_alpn, m_publicKey, m_shortId,
                             m_spiderX, m_path, m_host, m_grpcService, m_xhttpExtra,
                             m_encryption, m_dns, m_dns6}) {
        watchTextField(field);
    }
    for (QComboBox *combo : {m_fingerprint, m_flow, m_grpcMode, m_xhttpMode}) {
        connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), this,
                [this] { Q_EMIT settingChanged(); });
    }
    connect(m_security, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
        updateVisibility();
        Q_EMIT settingChanged();
        slotWidgetChanged();
    });
    connect(m_network, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
        updateVisibility();
        Q_EMIT settingChanged();
        slotWidgetChanged();
    });
    connect(m_port, qOverload<int>(&QSpinBox::valueChanged), this,
            [this] { Q_EMIT settingChanged(); });
    connect(m_mtu, qOverload<int>(&QSpinBox::valueChanged), this,
            [this] { Q_EMIT settingChanged(); });
    connect(m_ipv6, &QCheckBox::toggled, this, [this] {
        Q_EMIT settingChanged();
        slotWidgetChanged();
    });

    updateVisibility();
    updateLinkButton();
    slotWidgetChanged();
}

VlessSettingWidget::~VlessSettingWidget()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

void VlessSettingWidget::loadVpnSetting(const NetworkManager::VpnSetting::Ptr &setting)
{
    if (!setting) {
        return;
    }

    m_originalData = setting->data();
    m_originalSecrets = setting->secrets();

    m_address->setText(dataValue(m_originalData, QStringLiteral("address")));
    m_port->setValue(dataValue(m_originalData, QStringLiteral("port"), QString::number(DefaultPort)).toInt());
    m_uuid->setText(m_originalSecrets.value(QString::fromLatin1(SecretUuid)));

    setComboValue(m_security, dataValue(m_originalData, QStringLiteral("security"), QStringLiteral("none")));
    m_sni->setText(dataValue(m_originalData, QStringLiteral("sni")));
    setComboValue(m_fingerprint, dataValue(m_originalData, QStringLiteral("fingerprint")));
    m_alpn->setText(dataValue(m_originalData, QStringLiteral("alpn")));
    setComboValue(m_flow, dataValue(m_originalData, QStringLiteral("flow")));
    m_publicKey->setText(dataValue(m_originalData, QStringLiteral("reality-public-key")));
    m_shortId->setText(dataValue(m_originalData, QStringLiteral("reality-short-id")));
    m_spiderX->setText(dataValue(m_originalData, QStringLiteral("reality-spider-x")));

    setComboValue(m_network, dataValue(m_originalData, QStringLiteral("network"), QStringLiteral("tcp")));
    m_path->setText(dataValue(m_originalData, QStringLiteral("path")));
    m_host->setText(dataValue(m_originalData, QStringLiteral("host")));
    m_grpcService->setText(dataValue(m_originalData, QStringLiteral("grpc-service-name")));
    setComboValue(m_grpcMode, dataValue(m_originalData, QStringLiteral("network")) == QLatin1String("grpc")
                                  ? dataValue(m_originalData, QStringLiteral("mode"))
                                  : QString());
    setComboValue(m_xhttpMode, dataValue(m_originalData, QStringLiteral("network")) == QLatin1String("xhttp")
                                   ? dataValue(m_originalData, QStringLiteral("mode"))
                                   : QString());
    m_xhttpExtra->setText(dataValue(m_originalData, QStringLiteral("xhttp-extra")));

    m_encryption->setText(dataValue(m_originalData, QStringLiteral("encryption"), QStringLiteral("none")));
    m_hadMtu = m_originalData.contains(QStringLiteral("mtu"));
    m_mtu->setValue(dataValue(m_originalData, QStringLiteral("mtu"), QString::number(DefaultMtu)).toInt());
    m_dns->setText(dataValue(m_originalData, QStringLiteral("dns")));
    m_dns6->setText(dataValue(m_originalData, QStringLiteral("dns6")));
    m_hadIpv6 = m_originalData.contains(QStringLiteral("ipv6"));
    const QString ipv6 = dataValue(m_originalData, QStringLiteral("ipv6"), QStringLiteral("yes"));
    m_ipv6->setChecked(!(ipv6 == QLatin1String("no") || ipv6 == QLatin1String("false")
                         || ipv6 == QLatin1String("0")));
    m_uuidFlags = dataValue(m_originalData, QStringLiteral("uuid-flags"), QStringLiteral("0"));
}

QVariantMap VlessSettingWidget::setting() const
{
    NMStringMap data = m_originalData;
    for (const char *const *key = ManagedDataKeys; *key; ++key) {
        data.remove(QString::fromLatin1(*key));
    }

    const auto add = [&data](const char *key, const QString &value) {
        const QString clean = value.trimmed();
        if (!clean.isEmpty()) {
            data.insert(QString::fromLatin1(key), clean);
        }
    };

    const QString security = comboValue(m_security);
    const QString network = comboValue(m_network);
    add("address", m_address->text());
    add("port", QString::number(m_port->value()));
    add("encryption", m_encryption->text().isEmpty() ? QStringLiteral("none") : m_encryption->text());
    add("network", network);
    add("security", security);

    const bool tls = security == QLatin1String("tls");
    const bool reality = security == QLatin1String("reality");
    if (tls || reality) {
        add("sni", m_sni->text());
        add("fingerprint", comboValue(m_fingerprint));
    }
    if (tls) {
        add("alpn", m_alpn->text());
    }
    if ((tls || reality) && network == QLatin1String("tcp")) {
        add("flow", comboValue(m_flow));
    }
    if (reality) {
        add("reality-public-key", m_publicKey->text());
        add("reality-short-id", m_shortId->text());
        add("reality-spider-x", m_spiderX->text());
    }
    if (network == QLatin1String("ws") || network == QLatin1String("xhttp")
        || network == QLatin1String("httpupgrade")) {
        add("path", m_path->text());
        add("host", m_host->text());
    }
    if (network == QLatin1String("grpc")) {
        add("grpc-service-name", m_grpcService->text());
        add("mode", comboValue(m_grpcMode));
    } else if (network == QLatin1String("xhttp")) {
        add("mode", comboValue(m_xhttpMode));
        add("xhttp-extra", m_xhttpExtra->text());
    }

    if (m_mtu->value() != DefaultMtu || m_hadMtu) {
        add("mtu", QString::number(m_mtu->value()));
    }
    add("dns", m_dns->text());
    add("dns6", m_dns6->text());
    if (!m_ipv6->isChecked()) {
        add("ipv6", QStringLiteral("no"));
    } else if (m_hadIpv6) {
        add("ipv6", QStringLiteral("yes"));
    }
    add("uuid-flags", m_uuidFlags.isEmpty() ? QStringLiteral("0") : m_uuidFlags);

    NMStringMap secrets = m_originalSecrets;
    const QString uuid = m_uuid->text().trimmed();
    if (uuid.isEmpty()) {
        secrets.remove(QString::fromLatin1(SecretUuid));
    } else {
        secrets.insert(QString::fromLatin1(SecretUuid), uuid);
    }
    return vpnSettingMap(data, secrets);
}

bool VlessSettingWidget::isValid() const
{
    if (m_address->text().trimmed().isEmpty() || m_uuid->text().trimmed().isEmpty()) {
        return false;
    }
    if (comboValue(m_security) == QLatin1String("reality")) {
        return !m_sni->text().trimmed().isEmpty() && !m_publicKey->text().trimmed().isEmpty();
    }
    return true;
}

void VlessSettingWidget::startImport()
{
    const QString value = m_linkEdit->text().trimmed();
    if (value.startsWith(QLatin1String("vless://"), Qt::CaseInsensitive)) {
        setStatus(i18n("Reading the link…"));
        runCli({QStringLiteral("parse-link")}, value.toUtf8(), ImportKind::Link);
    } else if (value.startsWith(QLatin1String("https://"), Qt::CaseInsensitive)) {
        setStatus(i18n("Downloading the subscription…"));
        runCli({QStringLiteral("subscription"), QStringLiteral("add"), QStringLiteral("-"),
                QStringLiteral("--keyfile")},
               value.toUtf8(), ImportKind::Subscription);
    }
}

void VlessSettingWidget::runCli(const QStringList &arguments, const QByteArray &input, ImportKind kind)
{
    if (m_process) {
        return;
    }

    m_linkButton->setEnabled(false);
    auto *process = new QProcess(this);
    m_process = process;
    process->setProgram(QString::fromLocal8Bit(nm_vless_cli_path()));
    process->setArguments(arguments);
    connect(process, &QProcess::started, this, [process, input] {
        QByteArray request = input;
        if (!request.endsWith('\n')) {
            request.append('\n');
        }
        process->write(request);
        process->closeWriteChannel();
    });
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, process, kind](int exitCode, QProcess::ExitStatus exitStatus) {
                finishCli(process, exitCode, exitStatus, kind);
            });
    connect(process, &QProcess::errorOccurred, this,
            [this, process](QProcess::ProcessError error) {
                if (error == QProcess::FailedToStart && m_process == process) {
                    m_process = nullptr;
                    setStatus(i18n("Cannot start nm-vless. Check that it is installed."), true);
                    updateLinkButton();
                    process->deleteLater();
                }
            });
    process->start();
}

void VlessSettingWidget::finishCli(QProcess *process,
                                   int exitCode,
                                   QProcess::ExitStatus exitStatus,
                                   ImportKind kind)
{
    if (m_process != process) {
        return;
    }
    m_process = nullptr;
    const QByteArray output = process->readAllStandardOutput();
    const QByteArray errorOutput = process->readAllStandardError();
    process->deleteLater();
    updateLinkButton();

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        QString message = QString::fromLocal8Bit(errorOutput).trimmed();
        const qsizetype lastLine = message.lastIndexOf(QLatin1Char('\n'));
        if (lastLine >= 0) {
            message = message.mid(lastLine + 1).trimmed();
        }
        if (message.startsWith(QLatin1String("nm-vless: "))) {
            message.remove(0, QStringLiteral("nm-vless: ").size());
        }
        if (message.isEmpty()) {
            message = i18n("nm-vless exited with status %1.", exitCode);
        }
        setStatus(message, true);
        return;
    }

    if (kind == ImportKind::Link) {
        GError *error = nullptr;
        NMConnection *connection = nm_vless_connection_from_keyfile(output.constData(), &error);
        if (!connection) {
            const QString message = error ? QString::fromUtf8(error->message) : i18n("Invalid VLESS link.");
            g_clear_error(&error);
            setStatus(i18n("Cannot use this link: %1", message), true);
            return;
        }
        NetworkManager::VpnSetting::Ptr imported = toQtVpnSetting(nm_connection_get_setting_vpn(connection));
        const QString name = QString::fromUtf8(nm_connection_get_id(connection));
        g_object_unref(connection);
        if (!imported) {
            setStatus(i18n("The link did not contain VLESS settings."), true);
            return;
        }
        loadVpnSetting(imported);
        m_linkEdit->clear();
        setStatus(i18n("Filled in from “%1”.").arg(name));
        slotWidgetChanged();
        return;
    }

    GKeyFile *summary = g_key_file_new();
    GError *error = nullptr;
    if (!g_key_file_load_from_data(summary, output.constData(), output.size(), G_KEY_FILE_NONE, &error)) {
        const QString message = error ? QString::fromUtf8(error->message) : i18n("Invalid subscription response.");
        g_clear_error(&error);
        g_key_file_unref(summary);
        setStatus(i18n("Cannot add the subscription: %1", message), true);
        return;
    }

    gchar *name = g_key_file_get_string(summary, "subscription", "name", nullptr);
    const gint servers = g_key_file_get_integer(summary, "subscription", "added", nullptr)
                         + g_key_file_get_integer(summary, "subscription", "updated", nullptr)
                         + g_key_file_get_integer(summary, "subscription", "unchanged", nullptr);
    const QString subscriptionName = name ? QString::fromUtf8(name) : QString();
    g_free(name);
    g_key_file_unref(summary);
    m_linkEdit->clear();
    if (servers == 1) {
        setStatus(i18n("Subscription “%1” added: one server is now in the VPN list and will be updated automatically. You can close this dialog.")
                      .arg(subscriptionName));
    } else {
        setStatus(i18n("Subscription “%1” added: %2 servers are now in the VPN list and will be updated automatically. You can close this dialog.")
                      .arg(subscriptionName)
                      .arg(servers));
    }
}

void VlessSettingWidget::updateLinkButton()
{
    if (m_process) {
        m_linkButton->setEnabled(false);
        return;
    }
    const QString value = m_linkEdit->text().trimmed();
    const bool isLink = value.startsWith(QLatin1String("vless://"), Qt::CaseInsensitive);
    const bool isSubscription = value.startsWith(QLatin1String("https://"), Qt::CaseInsensitive)
                                && value.size() > 8;
    m_linkButton->setText(isSubscription ? i18n("Add Subscription") : i18n("Fill In"));
    m_linkButton->setEnabled(isLink || isSubscription);
}

void VlessSettingWidget::updateVisibility()
{
    const QString security = comboValue(m_security);
    const QString network = comboValue(m_network);
    const bool tls = security == QLatin1String("tls");
    const bool reality = security == QLatin1String("reality");
    const bool secured = tls || reality;
    const bool pathNetwork = network == QLatin1String("ws") || network == QLatin1String("xhttp")
                             || network == QLatin1String("httpupgrade");

    setFormRowVisible(m_securityForm, m_sni, secured);
    setFormRowVisible(m_securityForm, m_fingerprint, secured);
    setFormRowVisible(m_securityForm, m_alpn, tls);
    setFormRowVisible(m_securityForm, m_flow, secured && network == QLatin1String("tcp"));
    setFormRowVisible(m_securityForm, m_publicKey, reality);
    setFormRowVisible(m_securityForm, m_shortId, reality);
    setFormRowVisible(m_securityForm, m_spiderX, reality);

    setFormRowVisible(m_transportForm, m_path, pathNetwork);
    setFormRowVisible(m_transportForm, m_host, pathNetwork);
    setFormRowVisible(m_transportForm, m_grpcService, network == QLatin1String("grpc"));
    setFormRowVisible(m_transportForm, m_grpcMode, network == QLatin1String("grpc"));
    setFormRowVisible(m_transportForm, m_xhttpMode, network == QLatin1String("xhttp"));
    setFormRowVisible(m_transportForm, m_xhttpExtra, network == QLatin1String("xhttp"));
}

void VlessSettingWidget::setStatus(const QString &message, bool isError)
{
    m_linkStatus->setText(message);
    m_linkStatus->setStyleSheet(isError ? QStringLiteral("color: #b3261e;") : QString());
    m_linkStatus->setVisible(!message.isEmpty());
}

VlessAuthWidget::VlessAuthWidget(const NetworkManager::VpnSetting::Ptr &setting,
                                 const QStringList &hints,
                                 QWidget *parent)
    : SettingWidget(setting, hints, parent)
{
    auto *layout = new QFormLayout(this);
    layout->addRow(new QLabel(i18n("Enter the VLESS user ID to authenticate this VPN connection."), this));
    m_uuid = new QLineEdit(this);
    m_uuid->setEchoMode(QLineEdit::Password);
    m_uuid->setClearButtonEnabled(true);
    m_uuid->setPlaceholderText(i18n("UUID or VLESS user ID"));
    if (setting) {
        m_uuid->setText(setting->secrets().value(QString::fromLatin1(SecretUuid)));
    }
    layout->addRow(i18n("User ID"), m_uuid);
    connect(m_uuid, &QLineEdit::textChanged, this, [this] { slotWidgetChanged(); });
    watchChangedSetting();
    slotWidgetChanged();
}

QVariantMap VlessAuthWidget::setting() const
{
    NMStringMap secrets;
    const QString uuid = m_uuid->text().trimmed();
    if (!uuid.isEmpty()) {
        secrets.insert(QString::fromLatin1(SecretUuid), uuid);
    }
    QVariantMap result;
    result.insert(QStringLiteral("secrets"), QVariant::fromValue<NMStringMap>(secrets));
    return result;
}

bool VlessAuthWidget::isValid() const
{
    return !m_uuid->text().trimmed().isEmpty();
}
