#ifndef EXARGV_CONF_H_INCLUDED__
#define EXARGV_CONF_H_INCLUDED__

//[] 定義すると、WinMain 用に ExArgv_forWinMain を生成.
//#define EXARGV_FOR_WINMAIN

//[] 定義すると、wchar_t 用として生成. UNICODE 定義時は自動で定義される.
//#define EXARGV_USE_WCHAR

//[] 定義すると、EXARGV_USE_WCHAR を有効、ExArgv_wargvToU8等を利用可能に.
//#define EXARGV_USE_WCHAR_TO_UTF8
#define EXARGV_USE_WCHAR_TO_UTF8

//[] 定義され かつ UNICODE 未定義なら MBCS として2バイト目\文字対処を行う
//#define EXARGV_USE_MBC

//[] ワイルドカード指定を 1=有効  0=無効  未定義=デフォルト設定(1)
//#define EXARGV_USE_WC         1

//[] ワイルドカードon時に、ワイルドカード文字 ** があれば再帰検索に
//      1=する 0=しない 未定義=デフォルト設定(1)
//#define EXARGV_USE_WC_REC     1

//[] @レスポンスファイルを
//      1=有効   0=無効  未定義=デフォルト設定(0)
//#define EXARGV_USE_RESFILE    0
#define EXARGV_USE_RESFILE    1


//[] 簡易コンフィグ(レスポンス)ファイル入力を
//      1=有効  0=無効  未定義=デフォルト(0)
//   有効時は、win/dosなら .exe を .cfg に置換したパス名.
//             以外なら unix 系だろうで ~/.実行ファイル名.cfg
//#define EXARGV_USE_CONFIG     0
#define EXARGV_USE_CONFIG     1


//[] コンフィグファイル入力有効のとき、これを定義すれば、
//      コンフィグファイルの拡張子をこれにする.
#define EXARGV_CONFIG_EXT     ".ini"

//[] 定義すると、この名前の環境変数をコマンドライン文字列として利用.
//#define EXARGV_ENVNAME    "your_app_env_name"

//[] win環境のみ. argv[0] の実行ファイル名をフルパス化
//      1=有効  0=無効      未定義=デフォルト(0)
//   ※bcc,dmc,watcomは元からフルパスなので何もしません. のでvc,gcc向.
//#define EXARGV_USE_FULLPATH_ARGV0

//[] 定義すれば、filePath中の \ を / に置換.
//#define EXARGV_TOSLASH
#define EXARGV_TOSLASH

//[] 定義すれば、filePath中の / を \ に置換.
//#define EXARGV_TOBACKSLASH

//[] 定義すれば、/ もオプション開始文字とみなす.
//#define EXARGV_USE_SLASH_OPT

#endif
