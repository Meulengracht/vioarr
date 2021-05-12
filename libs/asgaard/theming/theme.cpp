/* ValiOS
 *
 * Copyright 2018, Philip Meulengracht
 *
 * This program is free software : you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ? , either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * ValiOS - Application Framework (Asgaard)
 *  - Contains the implementation of the application framework used for building
 *    graphical applications.
 */

#include "../include/theming/theme.hpp"
#include <libzip/ZipFile.h>
#include <libzip/streams/memstream.h>
#include <libzip/methods/Bzip2Method.h>
#include <fstream>

struct membuf : std::streambuf
{
    membuf(char *begin, char *end) : begin(begin), end(end)
    {
        this->setg(begin, begin, end);
    }

    virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in) override
    {
        if(dir == std::ios_base::cur)
        gbump(off);
        else if(dir == std::ios_base::end)
        setg(begin, end+off, end);
        else if(dir == std::ios_base::beg)
        setg(begin, begin+off, end);

        return gptr() - eback();
    }

    virtual pos_type seekpos(std::streampos pos, std::ios_base::openmode mode) override
    {
        return seekoff(pos - pos_type(off_type(0)), std::ios_base::beg, mode);
    }

    char *begin, *end;
};

namespace Asgaard {
namespace Theming {
    // declare the loader class which we want to keep private
    class ThemeLoader {
    public:
        ThemeLoader(const std::string& path)
        {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            m_buffer.reserve(size);
            if (!file.read(m_buffer.data(), size)) {
                // throw
            }
            
            m_wrapper = new membuf(m_buffer.data(), m_buffer.data() + size);
            m_stream = new std::istream(m_wrapper);
            m_zipHandle = ZipArchive::Create(m_stream, false);
        }
        ~ThemeLoader()
        {
            delete m_stream;
            delete m_wrapper;
        }

        ZipArchiveEntry::Ptr GetEntry(std::string& path) {
            if (!m_zipHandle) {
                return nullptr;
            }

            auto entry = m_zipHandle->GetEntry(path);
            if (!entry) {
                return nullptr;
            }

    #ifdef ASGAARD_THEME_PASSWORD
            if (entry->IsPasswordProtected()) {
                entry->SetPassword(std::string(ASGAARD_THEME_PASSWORD));
            }
    #else
            if (entry->IsPasswordProtected()) {
                return nullptr;
            }
    #endif

            return entry;
        }

    private:
        ZipArchive::Ptr   m_zipHandle;
        std::vector<char> m_buffer;
        membuf*           m_wrapper;
        std::istream*     m_stream;
    };
}
}

using namespace Asgaard;
using namespace Asgaard::Theming;


Theme::Theme(const std::string& themePack)
    : m_loader(nullptr)
{

}

Theme::~Theme()
{

}

void Theme::InitializeTheme()
{
    m_loader = std::make_unique<ThemeLoader>("$themes/default.pak");
    
    // push paths
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_CURSOR), "cursor16.png")); // 32, 40, 48

    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_CLOSE), "close16.png")); // 20, 24, 32
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_MAXIMIZE), "max16.png")); // 20, 24, 32
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_MINIMIZE), "min16.png")); // 20, 24, 32
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_APP_DEFAULT), "app16.png")); // 20, 24, 32

    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_SEARCH), "search-grey24.png"));
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_TERMINAL), "terminal64.png"));
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_EDITOR), "notepad64.png"));
    m_paths.insert(std::make_pair(static_cast<int>(Elements::IMAGE_GAME), "game64.png"));

    // load colors
    m_colors.insert(std::make_pair(static_cast<int>(Colors::DECORATION_FILL), Drawing::Color(0x0, 0x0C, 0x35, 0x33)));
    m_colors.insert(std::make_pair(static_cast<int>(Colors::DECORATION_TEXT), Drawing::Color(0xFF, 0xFF, 0xFF)));
}

Drawing::Image Theme::GetImage(enum Elements element)
{
    if (!m_loader) {
        InitializeTheme();
    }

    auto entry = m_loader->GetEntry(m_paths[static_cast<int>(element)]);
    if (!entry) {
        return Drawing::Image();
    }

    auto dataStream = entry->GetDecompressionStream();
    if (!dataStream) {
        return Drawing::Image();
    }

    Drawing::Image image(*dataStream, entry->GetSize());
    entry->CloseDecompressionStream();
    return image;
}

Drawing::Color Theme::GetColor(enum Colors color)
{
    if (!m_loader) {
        InitializeTheme();
    }

    return m_colors[static_cast<int>(color)];
}
