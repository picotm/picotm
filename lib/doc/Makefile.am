# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

EXTRA_DIST = Doxyfile.in

AM_DOXYGEN_FLAGS =

html-local:
	@DOXYGEN@ $(AM_DOXYGEN_FLAGS) Doxyfile

install-html-local:
	@MKDIR_P@ ${htmldir} && \
		cp -fr html/ ${htmldir}

uninstall-local:
	rm -fr ${htmldir}/html/
