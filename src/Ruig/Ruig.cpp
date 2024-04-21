/**
 *  @file Ruig.cpp
 *  @brief 簡易類似画像 検出・並替ツール
 *  @author Masashi Kitamura (tenka@6809.net)
 */
#include <stdio.h>
#include <stdlib.h>
#include "../ImgFmt/JpgDecoder.hpp"
#include "../misc/ExArgv.h"
#include "../misc/misc.hpp"
#include "../misc/PerfCnt.h"
#include "../misc/fks_file_time.h"
#include "NamedImg.hpp"
#include "ImgLoader.hpp"
#include "RuigMgr.hpp"


class App {
public:
    App()
        : allFlag_(false)
        , notsamedir_(false)
        , vflag_(false)
        , thumbMode_(true)
        , changeTimestamp_(false)
        , execType_(0)
        , batchMode_(false)
        , nearSame_(0)
        , prefix_("tmp\\img")
        , startTime_(0)
        , loadingTime_(0)
        , floadTime_(0)
        , jpgTime_(0)
        , sortIniTime_(0)
        , sortingTime_(0)
    {
    }

    /// メイン処理
    int main(int argc, char* argv[]) {
        if (argc < 2)
            return usage();
        startTime_ = PerfCnt_get();
        for (int i = 1; i < argc; ++i) {
            char const* p = argv[i];
            if (*p == '-') {
                if (scanOption(p))
                    return 1;
            }
        }
        if (vflag_) {
            fprintf(stderr, "images loading...\n");
        }
        for (int i = 1; i < argc; ++i) {
            char const* p = argv[i];
            if (*p != '-') {
                oneFile(p);
            }
        }
        fprintf(stderr, "\tload      %5u images.\n", unsigned(ruigMgr_.size()));
        loadingTime_ = PerfCnt_get();
        //fprintf(stderr, "\tLoad&Ini: %10.4f sec.\n", PERFCNT_TO_SEC(double(loadingTime_ - startTime_)));
        fprintf(stderr, "\tfileLoad: %10.4f sec.\n", PERFCNT_TO_SEC(double(floadTime_)));
        fprintf(stderr, "\tjpeg-dec: %10.4f sec.\n", PERFCNT_TO_SEC(double(jpgTime_)));
        fprintf(stderr, "\tsort-ini: %10.4f sec.\n", PERFCNT_TO_SEC(double(sortIniTime_)));

        body();
        fprintf(stderr, "\tnear|same %5d images.\n", unsigned(ruigMgr_.countNearSameFactors()));
        return 0;
    }

private:
    /// ヘルプ表示
    int usage() {
        printf("usage: ruig [-opts] jpeg-files...     # " __DATE__ " by tenk*\n");
        printf(" -?            help(this message).\n"
               " -c[PREFIX]    generate 'copy' batch.\n"
               "               (copy src.jpg [PREFIX]00000-00-0000x0000.jpg)\n"
               " -m[PREFIX]    generate 'move' batch.\n"
               "               (move src.jpg [PREFIX]00000-00-0000x0000.jpg)\n"
               " -n            print near picture file names. (default on)\n"
               " -s            print only same picture file names.\n"
               " -a            print all file names.\n"
               " -nd           match not same directory.\n"
               " -t            change timestamp by result order.\n"
               " -i            not use jpeg scale_denom.\n"
               );
        return 1;
    }

    /// オプション走査
    int scanOption(char const* opt) {
        char const* p = opt + 1;
        int  c = *(unsigned char*)p;
        ++p;
        switch (c) {
        case 'a':
            allFlag_    = (*p != '-');
            break;
        case 'c':
            execType_    = 1;
            if (*p)
                prefix_ = p;
            break;
        case 'm':
            execType_    = 2;
            if (*p)
                prefix_ = p;
            break;
        case 'n':
            if (*p == 'd')
                notsamedir_ = (*++p != '-');
            else if (*p == '\0')
                nearSame_ = 0;
            break;
        case 'i':
            thumbMode_ = 0;
            break;
        case 'o':
            oname_ = p;
            break;
        case 's':
            nearSame_ = 1;
            break;
        case 't':
            changeTimestamp_ = true;
            break;
        case 'v':
            vflag_ = (*p != '-');
            break;
        case '?':
            return usage();
        default:
            printf("ERROR: unkown option %s\n", opt);
            return 1;
        }
        return 0;
    }

    /// 1ファイル処理
    void oneFile(const char* name) {
        NamedImgPtr         imgPtr  = jpgLoad(name);
        BppCnvImg const&    rImg    = imgPtr->img();
        if (vflag_)
            fprintf(stderr, "%s %d*%d %dbpp\n", imgPtr->nameptr(), imgPtr->width(), imgPtr->height(), rImg.bpp());
        PerfCnt_t tm = PerfCnt_get();
        ruigMgr_.insert(imgPtr, buf_.size());
        sortIniTime_ += PerfCnt_get() - tm;
    }

    /// jpg画像ロード
    NamedImgPtr jpgLoad(const char* name) {
        buf_.clear();
        PerfCnt_t tm  = PerfCnt_get();
        bool      rc  = file_load(name, buf_);
        PerfCnt_t tm2 = PerfCnt_get();
        floadTime_   += tm2 - tm;
        if (!rc) {
            printf("%s : file load error\n", name);
            return NamedImgPtr();
        }
        NamedImgPtr p = imgDecoder_.jpgDecode(name, &buf_[0], buf_.size(), thumbMode_);
        jpgTime_ += PerfCnt_get() - tm2;
        return p;
    }

    /// すべて表示する場合
    struct PrintAll {
        PrintAll(App& rApp) : rApp_(rApp) {}
        bool operator()(RuigFactor const* factor) {
            rApp_.printFactor1(factor->majorNo(), factor->minorNo(), factor->subNo(), *factor);
            return false;
        }
    private:
        App&    rApp_;
    };

    /// すべて表示する場合
    struct PrintDebugLog {
        PrintDebugLog(App& rApp) : rApp_(rApp) {}
        bool operator()(RuigFactor const* factor) {
            char newName[2048];
            _snprintf(newName, sizeof newName, "%05d%02d-%02d-%04dx%04d"
                , factor->majorNo(), factor->minorNo(), factor->subNo(), factor->origWidth(), factor->origHeight());
            factor->debugPrintLog();
            return false;
        }
    private:
        App&    rApp_;
    };

    /// 近い画像のあるものだけ出力
    struct PrintNears {
        PrintNears(App& rApp) : rApp_(rApp), prevMajorNo_(0) { }
        bool operator()(RuigFactor const* factor) {
            if (factor->minorNo() > 0) {
                if (prevMajorNo_ != factor->majorNo() && prevMajorNo_ > 0)
                    printf("\n");
                rApp_.printFactor1(factor->majorNo(), factor->minorNo(), factor->subNo(), *factor);
                prevMajorNo_ = factor->majorNo();
            }
            return false;
        }
    private:
        App&        rApp_;
        uint32_t    prevMajorNo_;
    };

    /// 似た画像のあるものだけ出力
    struct PrintSames {
        PrintSames(App& rApp) : rApp_(rApp), prevMajorNo_(0), prevMinorNo_(0) { }
        bool operator()(RuigFactor const* factor) {
            if (factor->subNo() > 0) {
                if ((prevMajorNo_ != factor->majorNo() || prevMinorNo_ != factor->minorNo()) && prevMajorNo_ > 0)
                    printf("\n");
                rApp_.printFactor1(factor->majorNo(), factor->minorNo(), factor->subNo(), *factor);
                prevMajorNo_ = factor->majorNo();
                prevMinorNo_ = factor->minorNo();
            }
            return false;
        }
    private:
        App&        rApp_;
        uint32_t    prevMajorNo_;
        uint32_t    prevMinorNo_;
    };

    /// 違うフォルダの類似画像のみの標示
    struct PrintNears_NotMatchSameDir {
        typedef std::vector<RuigFactor const*>  Factors;

        PrintNears_NotMatchSameDir(App& rApp) : rApp_(rApp), prevMajorNo_(0) { }
        ~PrintNears_NotMatchSameDir() { checkPrint(); }

        bool operator()(RuigFactor const* factor) {
            if (factor->minorNo() > 0) {
                if (factor->majorNo() != prevMajorNo_) {
                    checkPrint();
                    factors_.clear();
                    prevMajorNo_ = factor->majorNo();
                }
                factors_.push_back(factor);
            }
            return false;
        }

        void checkPrint() {
            if (factors_.empty())
                return;
            RuigFactor const* factor = factors_[0];
            char const* pathname = factor->name().c_str();
            char const* basename = fname_baseName(pathname);
            size_t      len      = 0;
            if (basename > pathname)
                len = basename - pathname;

            bool samedir = true;
            for (size_t i = 1; i < factors_.size(); ++i) {
                char const* linkname = factors_[i]->name().c_str();
                if (strncmp(linkname, pathname, len) != 0) {
                    samedir = false;
                    break;
                }
            }
            if (!samedir) {
                for (size_t i = 0; i < factors_.size(); ++i) {
                    RuigFactor const* factor = factors_[i];
                    rApp_.printFactor1(factor->majorNo(), factor->minorNo(), factor->subNo(), *factor);
                }
                printf("\n");
            }
        }

    private:
        App&        rApp_;
        Factors     factors_;
        uint32_t    prevMajorNo_;
    };

    /// タイムスタンプを付け替える
    struct ChangeTimestamp {
        enum { SKIP_SEC = 60 };

        ChangeTimestamp(App& rApp)
            : rApp_(rApp), time_(0)
        {
            fks_file_time_t cur_time = fks_fileTimeFromCurrentTime();
            cur_time = cur_time /  FKS_FILE_TIME_FROM_SEC(SKIP_SEC) *  FKS_FILE_TIME_FROM_SEC(SKIP_SEC);
            time_ = cur_time - (rApp_.ruigMgr_.size() + 1) * FKS_FILE_TIME_FROM_SEC(SKIP_SEC);
        }

        bool operator()(RuigFactor const* factor) {
            char const* pathname = factor->name().c_str();
            fks_fileTimeSet(pathname, NULL, &time_, &time_);
            time_ += FKS_FILE_TIME_FROM_SEC(SKIP_SEC);
            return false;
        }

    private:
        App&            rApp_;
        fks_file_time_t time_;
    };

    /// 画像情報取得後の、各種処理
    void body() {
        if (vflag_)
            fprintf(stderr, "sorting...\n");

        if (ruigMgr_.run() == false) {
            fprintf(stderr, "no file\n");
            return;
        }

        sortingTime_ = PerfCnt_get();
        fprintf(stderr, "\tsort:     %10.4f sec.\n", PERFCNT_TO_SEC(double(sortingTime_ - loadingTime_)));

        if (vflag_)
            fprintf(stderr, "outputs...\n");

        if (changeTimestamp_)
            ruigMgr_.for_each(ChangeTimestamp(*this));
        else if (allFlag_)
            ruigMgr_.for_each(PrintAll(*this));
        else if (notsamedir_)
            ruigMgr_.for_each(PrintNears_NotMatchSameDir(*this));
        else if (nearSame_)
            ruigMgr_.for_each(PrintSames(*this));
        else
            ruigMgr_.for_each(PrintNears(*this));

        PerfCnt_t outputTime = PerfCnt_get();
        fprintf(stderr, "\toutput:   %10.4f sec.\n", PERFCNT_TO_SEC(double(outputTime - sortingTime_)));
        fprintf(stderr, "\tTOTAL:    %10.4f sec.\n", PERFCNT_TO_SEC(double(outputTime - startTime_  )));

        if (vflag_)
            ruigMgr_.for_each(PrintDebugLog(*this));
    }

    /// 画像一つの情報標示
    void printFactor1(uint32_t majorNo, uint32_t minorNo, uint32_t subNo, RuigFactor const& rFactor) {
        char    newName[2048];
        if (execType_) {
            char const* prefix = prefix_.c_str();
            if (!prefix)
                prefix = "";
            static char const* s_odrs[] = { "", "copy", "move" };
            _snprintf(newName, sizeof newName, "%s%05d%02d-%02d-%04dx%04d.jpg"
                , prefix, majorNo, minorNo, subNo, rFactor.origWidth(), rFactor.origHeight());
            printf("%s\t\"%s\"\t\"%s\"\n", s_odrs[execType_], rFactor.name().c_str(), newName);
        } else {
            printf("\"%s\"\n", rFactor.name().c_str());
        }
    }

    /// 画像一つの情報標示
    void execFactor1(uint32_t majorNo, uint32_t minorNo, uint32_t subNo, RuigFactor const& rFactor) {
        if (!execType_) {
            printf("\"%s\"\n", rFactor.name().c_str());
        } else {
            char    newName[2048];
            char const* prefix = prefix_.c_str();
            if (!prefix)
                prefix = "";
            static char const* s_odrs[] = { "", "copy", "move" };
            _snprintf(newName, sizeof newName, "%s%05d%02d-%02d-%04dx%04d.jpg"
                , prefix, majorNo, minorNo, subNo, rFactor.origWidth(), rFactor.origHeight());
          #if 1
            printf("%s\t\"%s\"\t\"%s\"\n", s_odrs[execType_], rFactor.name().c_str(), newName);
          #else
            if (batchMode_) {
                printf("%s\t\"%s\"\t\"%s\"\n", s_odrs[execType_], rFactor.name().c_str(), newName);
            } else {
                int rc = 0;
                if (execType_ == 1) {
                    rc = fks_file_copy(rFactor.name().c_str(), newName, FKS_FILE_TARGET_BACKUP);
                    if (rc < 0) {
                        fprintf(stderr, "ERROR: failed to copy from \"%s\" to \"%s\"\n", (rFactor.name().c_str(), newName );
                    }
                } else {
                    rc = fks_file_move(rFactor.name().c_str(), newName, FKS_FILE_TARGET_BACKUP);
                    if (rc < 0) {
                        fprintf(stderr, "ERROR: failed to move from \"%s\" to \"%s\"\n", (rFactor.name().c_str(), newName );
                    }
                }
            }
          #endif
        }
    }

    /// パス名の中のファイル名の位置を返す
    static char* fname_baseName(char const* adr)
    {
        char const* p = adr;
        while (*p) {
            if (*p == ':' || *p == '/' || *p == '\\')
                adr = (char const*)p + 1;
            ++p;
        }
        return (char*)adr;
    }

private:
    typedef std::vector<unsigned char>  Buf;
    Buf         buf_;
    ImgDecoder  imgDecoder_;
    RuigMgr     ruigMgr_;
    std::string oname_;
    std::string prefix_;
    bool        allFlag_;
    bool        notsamedir_;
    bool        vflag_;
    bool        thumbMode_;
    bool        changeTimestamp_;
    bool        batchMode_;
    int         execType_;
    int         nearSame_;
    PerfCnt_t   startTime_;
    PerfCnt_t   loadingTime_;
    PerfCnt_t   floadTime_;
    PerfCnt_t   jpgTime_;
    PerfCnt_t   sortIniTime_;
    PerfCnt_t   sortingTime_;
};



int main(int argc, char* argv[])
{
    ExArgv_conv(&argc, &argv);
    App     app;
    return app.main(argc, argv);
}

