project(
    'Kay',
    'c','cpp',
    version : '0.1.0',
    meson_version: '>= 0.62.0',
    default_options: [
        'warning_level=3',
        'buildtype=release',
        'cpp_std=c++20'])

cpp = meson.get_compiler('cpp')

pkg         = import('pkgconfig')
gl_dep      = dependency('gl')
glesv2_dep  = dependency('glesv2')
skia_dep    = dependency('Skia')
yoga_dep    = dependency('yoga', modules: ['yoga::yogacore'])

headers = run_command('find', './src', '-type', 'f', '-name', '*[.hpp,.h]', check : false).stdout().strip().split('\n')
sources = run_command('find', './src', '-type', 'f', '-name', '*[.cpp,.c]', check : false).stdout().strip().split('\n')

include_directories = ['./src']

kay = library(
    'Kay',
    sources : sources,
    include_directories : include_directories,
    dependencies : [
        skia_dep,
        gl_dep,
        glesv2_dep,
        yoga_dep
    ],
    install : false)

kay_dep = declare_dependency(
    dependencies: [skia_dep, yoga_dep],
    include_directories : include_directories,
    link_with : kay)

# -------------- EXAMPLES --------------

subdir('examples/kay-louvre')
subdir('examples/kay-srm')
