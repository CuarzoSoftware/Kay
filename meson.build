project(
    'Kay',
    'c','cpp',
    version : '0.1.0',
    meson_version: '>= 0.62.0',
    default_options: [
        'warning_level=2',
        'buildtype=release',
        'cpp_std=c++20'])

VERSION_MAJOR = meson.project_version().split('.')[0]
VERSION_MINOR = meson.project_version().split('.')[1]
VERSION_PATCH = meson.project_version().split('.')[2]
VERSION_BUILD = run_command('cat', './BUILD', check : false).stdout()

HEADERS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('includedir'))
ASSETS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('datadir'), 'Kay/assets')

add_project_arguments([
    '-DKAY_VERSION_MAJOR=@0@'.format(VERSION_MAJOR.to_int()),
    '-DKAY_VERSION_MINOR=@0@'.format(VERSION_MINOR.to_int()),
    '-DKAY_VERSION_PATCH=@0@'.format(VERSION_PATCH.to_int()),
    '-DKAY_VERSION_BUILD=@0@'.format(VERSION_BUILD.to_int()),
    '-DKAY_DEFAULT_ASSETS_PATH="@0@"'.format(ASSETS_INSTALL_PATH)
], language: 'cpp')

# -------------- DEPENDENCIES --------------

cpp             = meson.get_compiler('cpp')
pkg             = import('pkgconfig')
egl_dep         = dependency('egl')
gl_dep          = dependency('gl')
glesv2_dep      = dependency('glesv2')
skia_dep        = dependency('cz-skia')
cz_core_dep     = dependency('cz-core')
xkbcommon_dep   = dependency('xkbcommon')
yoga_dep        = dependency('yoga', modules: ['yoga::yogacore'])

deps = [
cz_core_dep,
  skia_dep,
  egl_dep,
  gl_dep,
  glesv2_dep,
  yoga_dep,
  xkbcommon_dep
]

if get_option('build_examples') or get_option('build_marco')
  wayland_client_dep  = dependency('wayland-client')
  wayland_cursor_dep  = dependency('wayland-cursor')
  wayland_egl_dep     = dependency('wayland-egl')
endif

if get_option('build_marco')
  deps += [
    wayland_client_dep,
    wayland_cursor_dep,
    wayland_egl_dep
  ]
endif

# -------------- SOURCES --------------

sources = run_command('find', './src/AK', '-type', 'f', '-name', '*[.cpp,.c,.h,.hpp]', check : false).stdout().strip().split('\n')
header_dirs = [
  ['./src/AK', 'AK'],
  ['./src/AK/nodes', 'AK/nodes'],
  ['./src/AK/effects', 'AK/effects'],
  ['./src/AK/events', 'AK/events'],
  ['./src/AK/utils', 'AK/utils'],
  ['./src/AK/input', 'AK/input']
]

if get_option('build_marco')
  header_dirs += [
    ['./src/Marco', 'Marco'],
    ['./src/Marco/private', 'Marco/private'],
    ['./src/Marco/roles', 'Marco/roles'],
    ['./src/Marco/nodes', 'Marco/nodes'],
    ['./src/Marco/protocols', 'Marco/protocols'],
    ['./src/Marco/input', 'Marco/input']
  ]
  sources += run_command('find', './src/Marco', '-type', 'f', '-name', '*[.cpp,.c,.h,.hpp]', check : false).stdout().strip().split('\n')
endif

foreach header_dir : header_dirs
    header_files = run_command('find', header_dir[0], '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(header_files, install_dir : join_paths(HEADERS_INSTALL_PATH, header_dir[1]))
endforeach

# -------------- LIBRARY --------------

kay = library(
    'Kay',
    sources : sources,
    include_directories : ['./src'],
    dependencies : deps,
    soversion: VERSION_MAJOR,
    install : true)

kay_dep = declare_dependency(
    dependencies: deps,
    include_directories : ['./src'],
    link_with : kay)

install_data(
    run_command('find', './assets', '-type', 'f', '-name', '*[.png,.jpg]', check : false).stdout().strip().split('\n'),
    install_dir: ASSETS_INSTALL_PATH
)

pkg.generate(
    kay,
    name: 'Kay',
    description: 'C++ GUI toolkit powered by Skia and Yoga.',
    version: meson.project_version(),
    filebase: 'Kay',
    requires: ['cz-skia'])

# -------------- EXAMPLES --------------

if get_option('build_examples')
    louvre_dep = dependency('cz-louvre')
    pixmap_dep = dependency('pixman-1')
    srm_dep = dependency('SRM')
    wl_mod = import('unstable-wayland')
    
    subdir('src/examples/kay-louvre')
    #subdir('src/examples/kay-srm')
    subdir('src/examples/kay-wayland-client')
    
    if get_option('build_marco')
      xdgkit_dep = dependency('XDGKit', fallback: ['XDGKit', 'xdgkit_dep'])
      subdir('src/examples/marco-toplevel')
      subdir('src/examples/marco-toplevel-custom-csd')
      subdir('src/examples/marco-subsurfaces')
      subdir('src/examples/marco-popup')
      subdir('src/examples/marco-layer-shell')
    endif
endif
