project(
  'c-vips-openslide', 'c',
  version : '0.0.1',
  default_options : [
    'buildtype=debug',
    'c_std=gnu99',
    'warning_level=2',
  ],
  # limited by Ubuntu 20.04
  meson_version : '>=0.53',
)
# Shared library version.  Follow SemVer rules.
soversion = '0.0.1'

conf = configuration_data()

# Calculate derived versions
version = meson.project_version()
suffixed_version = version
parts = (version + '.0.0.0').split('.')
windows_versioninfo = '@0@,@1@,@2@,@3@'.format(
  parts[0], parts[1], parts[2], parts[3]
)
conf.set_quoted('VERSION', version)
conf.set_quoted('SUFFIXED_VERSION', suffixed_version)
# unquoted versions
versions = {
  'VERSION': version,
  'SUFFIXED_VERSION': suffixed_version,
  'WINDOWS_VERSIONINFO': windows_versioninfo,
}

# Compiler flags
cc = meson.get_compiler('c')
cc_native = meson.get_compiler('c', native : true)
add_project_arguments(
  cc.get_supported_arguments(
    '-pedantic',
    '-pg',
  ),
  language : 'c'
)

# Dependencies
feature_flags = []
openslide_dep = dependency('openslide')
vips_dep = dependency('vips')

# Sauce
subdir('src')

# Tests
if not get_option('test').disabled()
  subdir('test')
endif
