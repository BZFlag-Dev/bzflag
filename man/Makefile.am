SUFFIXES = .6s .6

man_MANS = bzflag.6 bzfls.6 bzfrelay.6 bzfs.6

# replace tokens in source to make nroff source
.1s.1 .2s.2 .3s.3 .4s.4 .5s.5 .6s.6 .7s.7 .8s.8:
	cat $< | sed -e "s'INSTALL_DATA_DIR'$(INSTALL_DATA_DIR)'g" >$@

EXTRA_DIST = bzflag.6s bzfrelay.6s bzfs.6s bzfls.6s