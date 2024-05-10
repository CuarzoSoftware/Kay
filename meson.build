project(
    'Kay',
    'c','cpp',
    version : '0.1.0',
    meson_version: '>= 0.62.0',
    default_options: [
        'warning_level=3',
        'buildtype=release',
        'cpp_std=c++20'
	]
)

add_project_arguments([
    '-DSK_GL',
    '-DSK_GANESH'
], language: 'cpp')

cpp = meson.get_compiler('cpp')

pkg                      = import('pkgconfig')
wl_mod                   = import('unstable-wayland')
wayland_client_dep       = dependency('wayland-client')
wayland_egl_dep          = dependency('wayland-egl')
gl_dep                   = dependency('gl')
egl_dep                  = dependency('egl')
glesv2_dep               = dependency('glesv2')
pixman_dep               = dependency('pixman-1')
skia_dep                 = cpp.find_library('skia')

include_paths = [
    include_directories('./include'),
    include_directories('/usr/include/skia')
]

sources = run_command('find', './src', '-type', 'f', '-name', '*[.cpp,.c]', check : false).stdout().strip().split('\n')
sources += wl_mod.scan_xml(wl_mod.find_protocol('xdg-shell'))

kay = library(
    'Kay',
    sources : sources,
    include_directories : include_paths,
    dependencies : [
        wayland_client_dep,
        wayland_egl_dep,
        egl_dep,
        glesv2_dep,
        pixman_dep,
        skia_dep
    ],
    install : false)

kay_dep = declare_dependency(
    dependencies: [pixman_dep, wayland_client_dep, skia_dep],
    include_directories : include_paths,
    link_with : kay)

# -------------- EXAMPLES --------------

subdir('examples/basic')
