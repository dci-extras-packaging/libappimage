#pragma once

// system
#include <map>
#include <string>
#include <vector>
#include <memory>

// libraries
#include <appimage/core/AppImage.h>

namespace appimage {
    namespace utils {
        /**
         * Allows to identify and extract the resources (files) required to integrate an AppImage into the
         * desktop environment in an effective way.
         *
         * Using the `PayloadIterator::read` method on symlinks is not reliable as it's not supported on
         * AppImages of type 1 (blame on `libarchive`). To overcome this limitation two iterations over the
         * AppImage will be performed. One to resolve all the links entries and other to actually extract
         * the resources.
         */
        class ResourcesExtractor {
        public:
            explicit ResourcesExtractor(const core::AppImage& appImage);

            /**
             * @brief Read an entry into memory, if the entry is a link it will be resolved.
             * @return entry data
             * @throw PayloadIteratorError if the entry doesn't exists
             */
            std::vector<char> extract(const std::string& path) const;

            /**
             * @brief Read each entry into memory, if the entry is a link it will be resolved.
             * @return entries data
             * @throw PayloadIteratorError if some entry doesn't exists
             */
            std::map<std::string, std::vector<char>> extract(const std::vector<std::string>& paths) const;

            /**
             * Extract entries listed in 'first' member of the <targetsMap> iterator to the 'second' member
             * of the <targetsMap> iterator. Will resolve links to regular files.
             *
             * @param targetsMap
             */
            void extractTo(const std::map<std::string, std::string>& targetsMap) const;

            /**
             * @brief Read an entry into a std::string, if the entry is a link it will be resolved.
             * Should only be used in text files.
             *
             * @return entry data
             * @throw PayloadIteratorError if the entry doesn't exists
             */
            std::string extractText(const std::string& path) const;

            /**
             * @return path to the main desktop entry of the AppImage
             */
            std::string getDesktopEntryPath() const;

            /**
             * Icons are expected to be located in "usr/share/icons/" according to the FreeDesktop
             * Icon Theme Specification. This method look for entries in that path whose file name
             * matches to the iconName
             *
             * @param iconName
             * @return list of the icon entries paths
             */
            std::vector<std::string> getIconFilePaths(const std::string& iconName) const;

            /**
             * Mime-Type packages are xml files located usr/share/mime/packages according to the
             * Shared MIME-info Database specification.
             *
             * @param iconName
             * @return Mime-Type packages entries paths
             */
            std::vector<std::string> getMimeTypePackagesPaths() const;

        private:
            class Priv;
            std::shared_ptr<Priv> d;
        };
    }
}
