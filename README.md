Xpiks
=====

Cross-Platform Image Keywording Software

Official website: http://ribtoks.github.io/xpiks/

Translations: https://crowdin.com/project/xpiks/

Xpiks is a free and open source keywording and uploading tool for microstock photographers and illustrators. It was written from scratch with aim to make it more usefull, convenient and quicker than existing tools.

**Main features:**

- XMP/IPTC/EXIF metadata editing in images, vectors and videos
- FTP upload to predefined or custom hosts
- extensive keywording capabilities (autocompletion, spellchecking)
- keywords suggestion based on Shutterstock API, iStock, Fotolia or the local library
- automatic zipping of vector and preview for selected microstocks
- checks for potential problems before upload (insufficient resolution, missing metadata etc.)
- keywords presets to automate your work
- localized to other languages (Ukrainian, Portugese, Spanish, Catalan, Russian, French, German) 
- extensible with plugins
- works under Linux, OS X and Windows

This for is for Linux only. It contains changes and support files for creating a debian package for *Ubuntu 18*.
For more recent code and other platforms see original project : https://github.com/ribtoks/xpiks/

**How to build**

Please see the [BUILD.md](BUILD.md) file

**How to install**

Provided .deb file is created for *Ubuntu 18* but may work on other debian/Ubuntu derivatives 

Install the .deb file from latest release with all dependencies.

# Special instructions #

Install all recommended packages:
- libhunspell
- hunspell-en-us

**How to contribute**

Please see the [CONTRIBUTING.md](https://github.com/ribtoks/xpiks/blob/master/CONTRIBUTING.md) file
