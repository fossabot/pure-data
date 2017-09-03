/*****************************************************************************
 * Copyright 2017 Serge Poltavsky. All rights reserved.
 *
 * This file may be distributed under the terms of GNU Public License version
 * 3 (GPL v3) as defined by the Free Software Foundation (FSF). A copy of the
 * license should have been included with this file, or the project in which
 * this file belongs to. You may also find the details of GPL v3 at:
 * http://www.gnu.org/licenses/gpl-3.0.txt
 *
 * If you have any questions regarding the use of this file, feel free to
 * contact the author of this file, or the owner of the project in which
 * this file belongs to.
 *****************************************************************************/
#ifndef CEAMMC_SOUND_H
#define CEAMMC_SOUND_H

#include <boost/shared_ptr.hpp>
#include <m_pd.h>
#include <string>
#include <utility>
#include <vector>

namespace ceammc {
namespace sound {
    typedef std::pair<std::string, std::string> FormatDescription;
    typedef std::vector<FormatDescription> FormatList;
    class SoundFile;
    typedef boost::shared_ptr<SoundFile> SoundFilePtr;
    class SoundFilePlayer;
    typedef boost::shared_ptr<SoundFilePlayer> SoundFilePlayerPtr;

    class SoundFile {
        std::string fname_;

    public:
        SoundFile(const std::string& fname);
        virtual ~SoundFile();

        virtual std::string filename();
        virtual bool close() = 0;

        /**
         * @brief size in samples
         */
        virtual size_t sampleCount() const = 0;

        virtual size_t sampleRate() const = 0;

        virtual size_t channels() const = 0;

        /**
         * @brief read samples to given buffer
         * @param sz - destination buffer size
         * @param dest - pointer to destination
         * @return
         */
        virtual long read(t_word* dest, size_t sz, size_t channel, long offset = 0) = 0;

        virtual bool isOpened() const = 0;
    };

    class SoundFilePlayer;
    typedef SoundFilePtr (*loadFunc)(const std::string& path);
    typedef SoundFilePlayerPtr (*playerFunc)();
    typedef FormatList (*formatFunc)();
    struct LoaderDescr {
        LoaderDescr(const std::string& n, loadFunc f, formatFunc ff, playerFunc pf)
            : name(n)
            , func(f)
            , player(pf)
            , formats(ff)
        {
        }
        std::string name;
        loadFunc func;
        playerFunc player;
        formatFunc formats;
        bool operator==(const LoaderDescr& l);
    };

    class SoundFileLoader {
    public:
        static bool registerLoader(const LoaderDescr& l);
        static FormatList supportedFormats();

        static SoundFilePtr open(const std::string& path);
        static SoundFilePlayerPtr player();

    private:
        typedef std::vector<LoaderDescr> LoaderList;
        static LoaderList& loaders(); // singleton
    };

    class SoundFilePlayer {
    protected:
        std::string path_;

    public:
        SoundFilePlayer();
        virtual ~SoundFilePlayer();

        virtual bool open(const std::string& filename) = 0;
        virtual bool close() = 0;

        virtual size_t sampleCount() const = 0;
        virtual size_t sampleRate() const = 0;
        virtual size_t channels() const = 0;

        std::string filename() const;

        virtual bool isOpened() const = 0;
        virtual bool seek(size_t offset) = 0;
        virtual size_t tell() const = 0;
        virtual size_t read(t_sample** dest, size_t n) = 0;
    };
}
}

#endif // CEAMMC_SOUND_H
