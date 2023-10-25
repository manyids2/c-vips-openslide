# Shared library version.  Follow SemVer rules.
soversion = '0.0.1'
openslide_dep = dependency('openslide')
vips_dep = dependency('vips')

executable('c-vips-openslide',
           'ops.c',
           'slide.c',
           'resize.c',
           'main.c',
           dependencies: [openslide_dep, vips_dep],
           install : true)
