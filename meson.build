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

HEADERS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('includedir'), 'AK')

# -------------- DEPENDENCIES --------------

cpp             = meson.get_compiler('cpp')
pkg             = import('pkgconfig')
egl_dep         = dependency('egl')
gl_dep          = dependency('gl')
glesv2_dep      = dependency('glesv2')
skia_dep        = dependency('Skia')
xkbcommon_dep   = dependency('xkbcommon')
yoga_dep        = dependency('yoga', modules: ['yoga::yogacore'])

# -------------- HEADERS --------------

header_dirs = [
    ['./src/AK', ''],
    ['./src/AK/nodes', 'nodes'],
    ['./src/AK/effects', 'effects'],
    ['./src/AK/events', 'events'],
    ['./src/AK/utils', 'utils'],
    ['./src/AK/input', 'input']
]

foreach header_dir : header_dirs
    header_files = run_command('find', header_dir[0], '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(header_files, install_dir : join_paths(HEADERS_INSTALL_PATH, header_dir[1]))
endforeach

# -------------- LIBRARY --------------

kay = library(
    'Kay',
    sources : run_command('find', './src/AK', '-type', 'f', '-name', '*[.cpp,.c,.h,.hpp]', check : false).stdout().strip().split('\n'),
    include_directories : ['./src'],
    dependencies : [
        skia_dep,
        egl_dep,
        gl_dep,
        glesv2_dep,
        yoga_dep,
        xkbcommon_dep
    ],
    soversion: VERSION_MAJOR,
    install : true)

kay_dep = declare_dependency(
    dependencies: [skia_dep, yoga_dep, egl_dep, gl_dep, glesv2_dep],
    include_directories : ['./src'],
    link_with : kay)

pkg.generate(
    kay,
    name: 'Kay',
    description: 'C++ GUI toolkit powered by Skia and Yoga.',
    version: meson.project_version(),
    filebase: 'Kay',
    requires: ['Skia', 'egl', 'gl', 'glesv2'])

# -------------- EXAMPLES --------------

if get_option('build_examples')
    louvre_dep = dependency('Louvre')
    pixmap_dep = dependency('pixman-1')
    srm_dep = dependency('SRM')
    wl_mod = import('unstable-wayland')
    wayland_client_dep = dependency('wayland-client')
    wayland_egl_dep = dependency('wayland-egl')
    subdir('src/examples/kay-louvre')
    subdir('src/examples/kay-srm')
    subdir('src/examples/kay-wayland-client')
endif
