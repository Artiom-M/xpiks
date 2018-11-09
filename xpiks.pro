TEMPLATE = subdirs

SUBDIRS = \
        src/libxpks_stub/libxpks_stub.pro  \
        vendors/cpp-libface/libface-project/libface.pro \
        ssdll \
        app

with-video {

SUBDIRS = \
        vendors/libthmbnlr/libthmbnlr.pro \   # Video thumbnail creator
        src/libxpks_stub/libxpks_stub.pro  \
        vendors/cpp-libface/libface-project/libface.pro \
        ssdll \
        app
	
}

# where to find the sub projects - give the folders
ssdll.subdir = vendors/ssdll/src/ssdll/
app.subdir = src/xpiks-qt


app.depends = libxpks_stub libface ssdll

with-video {
	app.depends = libthmbnlr libxpks_stub libface ssdll
}

CONFIG += ordered