# laf documentation

*laf* is a C++ library to create desktop applications for Windows,
macOS, and Linux. It was mainly developed for
[Aseprite](https://www.aseprite.org/) but we want to make it available
for other developers in such a way that they can create applications
too.

This "framework" (or library, or set of libraries) is still under
development, but we'd like to start stabilizing the main API in a near
future. The objective is to create a library as small as possible, and
leaving parts that are not required to be integrated, to other
libraries. For example, signal and slots can be used with [other
libraries](https://julienjorge.medium.com/testing-c-signal-slot-libraries-1994eb120826),
or the clipboard management with [dacap/clip](https://github.com/dacap/clip),
etc. Even more, parts of *laf* might be separated to other libraries
in the future if they are highly valuable on their own.
