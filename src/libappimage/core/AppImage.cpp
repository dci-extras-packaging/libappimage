// system
#include <iostream>
#include <algorithm>


// local
#include "appimage/core/AppImage.h"
#include "utils/MagicBytesChecker.h"
#include "utils/ElfFile.h"

namespace appimage {
    namespace core {

        /**
         * Implementation of the opaque pointer patter for the appimage class
         * see https://en.wikipedia.org/wiki/Opaque_pointer
         */
        class AppImage::Private {
        public:
            std::string path;
            AppImageFormat format = AppImageFormat::INVALID;

            explicit Private(const std::string& path);

            static AppImageFormat getFormat(const std::string& path);

        };

        AppImage::AppImage(const std::string& path) : d(new Private(path)) {
        }

        const std::string& AppImage::getPath() const {
            return d->path;
        }

        AppImageFormat AppImage::getFormat() const {
            return d->format;
        }

        AppImageFormat AppImage::getFormat(const std::string& path) {
            return Private::getFormat(path);
        }

        AppImage::Private::Private(const std::string& path) : path(path) {
            format = getFormat(path);

            if (format == AppImageFormat::INVALID)
                throw core::AppImageError("Unknown AppImage format: " + path); // FIXME: This should not be an error, and should not be printed unless in debug mode
        }

        AppImageFormat AppImage::Private::getFormat(const std::string& path) {
            utils::MagicBytesChecker magicBytesChecker(path);

            if (!magicBytesChecker.hasElfSignature())
                return AppImageFormat::INVALID;

            if (magicBytesChecker.hasAppImageType1Signature())
                return AppImageFormat::TYPE_1;

            if (magicBytesChecker.hasAppImageType2Signature())
                return AppImageFormat::TYPE_2;

            if (magicBytesChecker.hasIso9660Signature()) {
                std::cerr << "WARNING: " << path << " seems to be a Type 1 AppImage without magic bytes." << std::endl;
                return AppImageFormat::TYPE_1;
            }

            return AppImageFormat::INVALID;
        }

        AppImage::~AppImage() = default;

        PayloadIterator AppImage::files() const {
            return PayloadIterator(*this);
        }

        off_t AppImage::getPayloadOffset() const {
            utils::ElfFile elf(d->path);

            return elf.getSize();
        }

        bool AppImage::operator==(const AppImage& rhs) const {
            return d == rhs.d;
        }

        bool AppImage::operator!=(const AppImage& rhs) const {
            return !(rhs == *this);
        }

        AppImage& AppImage::operator=(const AppImage& other) = default;

        AppImage::AppImage(const AppImage& other) = default;
    }
}
