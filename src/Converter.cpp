#include "Converter.h"
#include "Logger.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

Q_LOGGING_CATEGORY(lcConverter, "le.converter")

namespace LE {

void Converter::convert(const ConversionParams& params)
{
    qCInfo(lcConverter) << "Conversion start:" << params.inputPath << "->" << params.outputPath;

    if (!params.inputPath.endsWith(".m3u", Qt::CaseInsensitive) &&
        !params.inputPath.endsWith(".m3u8", Qt::CaseInsensitive))
    {
        throw std::runtime_error("Unsupported file type. Expected .m3u or .m3u8.");
    }

    if (params.inputPath.endsWith(".m3u", Qt::CaseInsensitive)) {
        convertM3uToM3u8(params);
    } else {
        convertM3u8ToM3u(params);
    }

    qCInfo(lcConverter) << "Conversion complete:" << params.outputPath;
}

// ─── M3U → M3U8 ────────────────────────────────────────────────────────────

void Converter::convertM3uToM3u8(const ConversionParams& params)
{
    if (params.basePath.isEmpty()) {
        throw std::runtime_error("Base path is required for M3U → M3U8 conversion.");
    }

    const QString base = normalizePath(params.basePath);
    const QString outputName = QFileInfo(params.outputPath).completeBaseName();

    QFile inFile(params.inputPath);
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCCritical(lcConverter) << "Failed to open input file:" << params.inputPath;
        throw std::runtime_error("Cannot open input file: " + params.inputPath.toStdString());
    }

    QFile outFile(params.outputPath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qCCritical(lcConverter) << "Failed to open output file:" << params.outputPath;
        throw std::runtime_error("Cannot open output file: " + params.outputPath.toStdString());
    }

    QTextStream in(&inFile);
    QTextStream out(&outFile);
    in.setEncoding(QStringConverter::Utf8);
    out.setEncoding(QStringConverter::Utf8);

    out << "#EXTM3U\n";
    out << "#" << outputName << ".m3u8\n";

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        line = stripLeadingMusicPrefix(line);
        line = normalizePath(line);

        const QString fullPath = base + '/' + line;
        out << fullPath << '\n';
    }

    qCDebug(lcConverter) << "M3U→M3U8 written to:" << params.outputPath;
}

// ─── M3U8 → M3U ────────────────────────────────────────────────────────────

void Converter::convertM3u8ToM3u(const ConversionParams& params)
{
    QString customBase;

    if (params.locationMode == LocationMode::Custom) {
        if (params.basePath.isEmpty()) {
            throw std::runtime_error("Custom base path is required for custom location mode.");
        }
        customBase = normalizePath(params.basePath);
    }

    QFile inFile(params.inputPath);
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCCritical(lcConverter) << "Failed to open input file:" << params.inputPath;
        throw std::runtime_error("Cannot open input file: " + params.inputPath.toStdString());
    }

    QFile outFile(params.outputPath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qCCritical(lcConverter) << "Failed to open output file:" << params.outputPath;
        throw std::runtime_error("Cannot open output file: " + params.outputPath.toStdString());
    }

    QTextStream in(&inFile);
    QTextStream out(&outFile);
    in.setEncoding(QStringConverter::Utf8);
    out.setEncoding(QStringConverter::Utf8);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        const QString originalPath = normalizePath(line);

        if (params.locationMode == LocationMode::Keep) {
            out << originalPath << '\n';
        } else {
            const QString filename = QFileInfo(originalPath).fileName();
            out << customBase << '/' << filename << '\n';
        }
    }

    qCDebug(lcConverter) << "M3U8→M3U written to:" << params.outputPath;
}

// ─── Helpers ────────────────────────────────────────────────────────────────

QString Converter::normalizePath(const QString& path)
{
    QString normalized = path;
    normalized.replace('\\', '/');

    while (normalized.endsWith('/')) {
        normalized.chop(1);
    }

    return normalized;
}

QString Converter::stripLeadingMusicPrefix(const QString& line)
{
    constexpr QLatin1StringView prefix{"Music/"};
    if (line.startsWith(prefix)) {
        return line.sliced(prefix.size());
    }
    return line;
}

} // namespace LE