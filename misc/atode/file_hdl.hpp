// ---------------------------------------------------------------------------
// c++ の時の、ハンドルをラップしたクラス

#ifdef __cplusplus

class FileHdl {
public:
    enum OpenMode { R,W,RP,WP };

    FileHdl() : hdl_(FILE_ERR_HDL) {}
    explicit FileHdl(const TCHAR* nm, OpenMode md=R) : hdl_(FILE_ERR_HDL) { open(nm, md); }
    ~FileHdl() { close(); }

    bool            is_open() const { return hdl_ != FILE_ERR_HDL; }
    bool            open_r (const TCHAR* nm) { hdl_ = file_open_r (nm); return hdl_ != FILE_ERR_HDL; }
    bool            open_w (const TCHAR* nm) { hdl_ = file_open_w (nm); return hdl_ != FILE_ERR_HDL; }
    bool            open_rp(const TCHAR* nm) { hdl_ = file_open_rp(nm); return hdl_ != FILE_ERR_HDL; }
    bool            open_wp(const TCHAR* nm) { hdl_ = file_open_wp(nm); return hdl_ != FILE_ERR_HDL; }

    bool            open(const TCHAR* nm, OpenMode md=R) {
                        assert(hdl_ == FILE_ERR_HDL);
                        switch (md) {
                        case R : hdl_ = file_open_r (nm); break;
                        case W : hdl_ = file_open_w (nm); break;
                        case RP: hdl_ = file_open_rp(nm); break;
                        case WP: hdl_ = file_open_wp(nm); break;
                        default: assert(0);
                        }
                        return hdl_ != FILE_ERR_HDL;
                    }

    void            close() { if (hdl_ != FILE_ERR_HDL) {file_close(hdl_);}  hdl_ = FILE_ERR_HDL; }

    file_size_t   size() { return file_getSize(hdl_); }
    size_t          read(void* b, size_t sz) { return file_read(hdl_, b, sz); }
    size_t          write(const void* b, size_t sz) { return file_write(hdl_, b, sz); }
    int             flush() { return file_flush(hdl_); }
    file_size_t   seek(file_size_t ofs, int mode) { return file_seek(hdl_, ofs, mode); }
    file_size_t   tell() { return file_tell(hdl_); }

    file_size_t   getTime(file_time_t* pCreat, file_time_t* pLastAcs, file_time_t* pLastWrt) {
                        return file_getTime(hdl_, pCreat, pLastAcs, pLastWrt);
                    }
    int             setTime(file_time_t creatTim, file_time_t lastAcs, file_time_t lastWrt) {
                        return file_setTime(hdl_, creatTim, lastAcs, lastWrt);
                    }

    size_t          puts(const TCHAR* str) { return file_puts(str, hdl_); }
    template<class STRING>
    size_t          puts(const STRING& str) { return file_puts(str.c_str(), hdl_); }

    size_t          printf(const TCHAR* fmt, ...) {
                        va_list  a;
                        va_start(a,fmt);
                        size_t n = file_vprintf(hdl_, fmt, a);
                        va_end(a);
                        return n;
                    }

    FILE_HDL        get_handle() const { return hdl_;}

private:
    FILE_HDL        hdl_;
};

#endif  // __cplusplus


#ifdef FILE_HDL_NS
}
#endif

