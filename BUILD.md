**Prerequisits**

- here assumed that you have generic dev pakages installed
- install dependenices listed in debian/control file including `hunspell` library from recommended list 
- install development packages listed in debian/control file

**Automatic build instructions:**

- Go to the sources root directory (where you can find `xpiks.pro` file)
- `debuild -i -us -uc -b`

**Manual build**

- open main project (`src/xpiks-qt/xpiks-qt.pro`) in Qt Creator, execute `Run qmake`, execute `Build`
- you can open, build and run tests project (`src/xpiks-tests/xpiks-tests.pro`) additionally

If you want to create Xpiks executable for everyday usage, before each `Run qmake` step described below, select `Release` in Qt Creator. Select `Debug` (the default) in all other cases.

***Additional notes***

This guide was written based on *Ubuntu 18*. In other versions or debain derivatives packages can have slightly different names.

