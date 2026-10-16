/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "vlessplugin.h"
#include "vlesssettingwidget.h"

#include "nm-vless-common.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <QFile>

namespace {
constexpr qsizetype MaxImportSize = 64 * 1024;

NetworkManager::VpnSetting::Ptr ensureSetting(const NetworkManager::VpnSetting::Ptr &setting)
{
    return setting ? setting : NetworkManager::VpnSetting::Ptr(new NetworkManager::VpnSetting);
}
}

K_PLUGIN_CLASS_WITH_JSON(VlessUiPlugin, "plasmanetworkmanagement_vlessui.json")

VlessUiPlugin::VlessUiPlugin(QObject *parent, const QVariantList &arguments)
    : VpnUiPlugin(parent, arguments)
{
    KLocalizedString::setApplicationDomain(QStringLiteral("NetworkManager-vless"));
}

VlessUiPlugin::~VlessUiPlugin() = default;

SettingWidget *VlessUiPlugin::widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
{
    return new VlessSettingWidget(ensureSetting(setting), parent);
}

SettingWidget *VlessUiPlugin::askUser(const NetworkManager::VpnSetting::Ptr &setting,
                                     const QStringList &hints,
                                     QWidget *parent)
{
    return new VlessAuthWidget(ensureSetting(setting), hints, parent);
}

QString VlessUiPlugin::suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const
{
    const QString name = connection ? connection->id().trimmed() : QString();
    return (name.isEmpty() ? QStringLiteral("vless") : name) + QStringLiteral(".vless");
}

QStringList VlessUiPlugin::supportedFileExtensions() const
{
    return {QStringLiteral("*.vless"), QStringLiteral("*.txt")};
}

VpnUiPlugin::ImportResult VlessUiPlugin::importConnectionSettings(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return ImportResult::fail(i18n("Cannot read the file: %1", file.errorString()));
    }

    const QByteArray contents = file.read(MaxImportSize + 1);
    if (file.error() != QFileDevice::NoError) {
        return ImportResult::fail(i18n("Cannot read the file: %1", file.errorString()));
    }
    if (contents.size() > MaxImportSize || contents.contains('\0')
        || !g_utf8_validate(contents.constData(), contents.size(), nullptr)) {
        return ImportResult::fail(i18n("The file is too large or is not valid UTF-8."));
    }

    gchar *link = nm_vless_find_link(contents.constData());
    if (!link) {
        return ImportResult::fail(i18n("The file does not contain a vless:// link."));
    }

    GError *error = nullptr;
    gchar *keyfile = nm_vless_parse_link_sync(link, nullptr, &error);
    g_free(link);
    if (!keyfile) {
        const QString message = error ? QString::fromUtf8(error->message) : i18n("Invalid VLESS link.");
        g_clear_error(&error);
        return ImportResult::fail(i18n("Cannot use this link: %1", message));
    }

    NMConnection *connection = nm_vless_connection_from_keyfile(keyfile, &error);
    g_free(keyfile);
    if (!connection) {
        const QString message = error ? QString::fromUtf8(error->message) : i18n("Invalid VLESS link.");
        g_clear_error(&error);
        return ImportResult::fail(i18n("Cannot use this link: %1", message));
    }

    return ImportResult::pass(connection);
}

#include "vlessplugin.moc"
