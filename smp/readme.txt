src_???(src_img, src_paint等)フォルダに jpg,png 画像をほりこんで、適当に変換を試し見るためのフォルダ

変換元画像を src_??? に適当に入れる
  src_img       一般用
  src_paint     ４隅ペイント用

予め visual studio で x64 release でコンパイルした場合は
コマンドライン(dos窓)にて、
  set1st.bat r64
を実行、そのあと cnv_????.bat を実行して画像変換.

bmptg/mk_????.bat でコンパイルしたものの場合は
 set1st.bat
を実行後、同様に.
