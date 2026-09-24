<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<!--
  Simplified Chinese translation for FenSu Cloud Album.

  Qt normally generates this file with lupdate, which walks the sources for
  qsTr() and tr() calls. That tool is not available on the machine where this
  was written, so the file is maintained by hand. It is read by lrelease at
  build time (see qt_add_translations in CMakeLists.txt) and loaded at
  startup by main.cpp.

  When adding a translatable string:
    1. wrap it in qsTr("...") in QML or tr("...") in C++
    2. add a <message> here with the same <source> text
    3. leave <translation> empty to fall back to the English source text

  Keep the <source> strings byte identical to the ones in the code, otherwise
  the lookup fails silently and the user sees English. Note that the ellipsis
  in the code is the escape \u2026 inside a C++/QML string literal; here it is
  written as the actual character, because a .ts file is XML and \u2026 has no
  meaning in XML.
-->
<TS version="2.1" language="zh_CN" sourcelanguage="en">
    <context>
        <name>Main</name>
        <message>
            <source>FenSu Cloud Album</source>
            <translation>芬速云相册</translation>
        </message>
        <message>
            <source>Preparing your library…</source>
            <translation>正在准备媒体库…</translation>
        </message>
    </context>

    <context>
        <name>BottomNavBar</name>
        <message>
            <source>Albums</source>
            <translation>相册</translation>
        </message>
        <message>
            <source>All</source>
            <translation>全部</translation>
        </message>
        <message>
            <source>Memories</source>
            <translation>回忆</translation>
        </message>
    </context>

    <context>
        <name>AlbumsPage</name>
        <message>
            <source>Albums</source>
            <translation>相册</translation>
        </message>
        <message>
            <source>No albums yet</source>
            <translation>还没有相册</translation>
        </message>
        <message>
            <source>Import photos or connect a WebDAV account to get started.</source>
            <translation>导入照片或连接 WebDAV 账号即可开始。</translation>
        </message>
        <message>
            <source>Preparing local storage…</source>
            <translation>正在准备本地存储…</translation>
        </message>
        <message>
            <source>1 item</source>
            <translation>1 项</translation>
        </message>
        <message>
            <source>%1 items</source>
            <translation>%1 项</translation>
        </message>
    </context>

    <context>
        <name>AllPhotosPage</name>
        <message>
            <source>All photos</source>
            <translation>所有照片</translation>
        </message>
        <message>
            <source>No photos yet</source>
            <translation>还没有照片</translation>
        </message>
        <message>
            <source>Photos and videos you add will appear here.</source>
            <translation>你添加的照片和视频会显示在这里。</translation>
        </message>
        <message>
            <source>1 item</source>
            <translation>1 项</translation>
        </message>
        <message>
            <source>%1 items</source>
            <translation>%1 项</translation>
        </message>
    </context>

    <context>
        <name>MemoriesPage</name>
        <message>
            <source>Memories</source>
            <translation>回忆</translation>
        </message>
        <message>
            <source>No memories yet</source>
            <translation>还没有回忆</translation>
        </message>
        <message>
            <source>Once you have photos from earlier days, they will show up here automatically.</source>
            <translation>当你有往日的照片后，它们会自动出现在这里。</translation>
        </message>
        <message>
            <source>This year</source>
            <translation>今年</translation>
        </message>
        <message>
            <source>%1 year(s) ago</source>
            <translation>%1 年前</translation>
        </message>
    </context>

    <context>
        <name>AlbumCard</name>
        <message>
            <source>1 item</source>
            <translation>1 项</translation>
        </message>
        <message>
            <source>%1 items</source>
            <translation>%1 项</translation>
        </message>
    </context>

    <context>
        <name>AppController</name>
        <message>
            <source>Unable to resolve a writable application data location.</source>
            <translation>无法确定可写入的应用数据目录。</translation>
        </message>
        <message>
            <source>Unable to create application data directory: %1</source>
            <translation>无法创建应用数据目录：%1</translation>
        </message>
        <message>
            <source>FenSu Cloud Album %1
Platform: %2
Storage: %3
Database: %4</source>
            <translation>芬速云相册 %1
平台：%2
存储：%3
数据库：%4</translation>
        </message>
        <message>
            <source>(not initialised)</source>
            <translation>（未初始化）</translation>
        </message>
    </context>
</TS>