TEMPLATE = subdirs

SUBDIRS = \
        vendors/libthmbnlr/libthmbnlr.pro \   # sub-project names
        src/libxpks_stub/libxpks_stub.pro  \
        vendors/cpp-libface/libface-project/libface.pro \
        ssdll \
        app

# where to find the sub projects - give the folders
ssdll.subdir = vendors/ssdll/src/ssdll/
app.subdir = src/xpiks-qt


# what subproject depends on others
app.depends = libthmbnlr libxpks_stub libface ssdll

CONFIG += ordered