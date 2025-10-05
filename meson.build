project(
    'cz-kay',
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
cz_core_dep     = dependency('cz-core')
cz_skia_dep     = dependency('cz-skia')
cz_ream_dep     = dependency('cz-ream')
xkbcommon_dep   = dependency('xkbcommon')
yoga_dep        = dependency('yoga', modules: ['yoga::yogacore'])

deps = [
  cz_core_dep,
  cz_skia_dep,
  cz_ream_dep,
  yoga_dep,
  xkbcommon_dep
]

# -------------- SOURCES --------------

sources = run_command('find', './src/CZ/AK', '-type', 'f', '-name', '*[.cpp,.c,.h,.hpp]', check : false).stdout().strip().split('\n')
header_dirs = [
  ['./src/CZ/AK', 'CZ/AK'],
  ['./src/CZ/AK/Nodes', 'CZ/AK/Nodes'],
  ['./src/CZ/AK/Effects', 'CZ/AK/Effects'],
  ['./src/CZ/AK/Events', 'CZ/AK/Events'],
  ['./src/CZ/AK/Input', 'CZ/AK/Input']
]

foreach header_dir : header_dirs
    header_files = run_command('find', header_dir[0], '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(header_files, install_dir : join_paths(HEADERS_INSTALL_PATH, header_dir[1]))
endforeach

# -------------- LIBRARY --------------

kay = library(
    'cz-kay',
    sources : sources,
    include_directories : ['src'],
    dependencies : deps,
    soversion: VERSION_MAJOR,
    install : true)

kay_dep = declare_dependency(
    dependencies: deps,
    include_directories : ['src', 'src/CZ/AK'],
    link_with : kay)

install_data(
    run_command('find', './assets', '-type', 'f', '-name', '*[.png,.jpg]', check : false).stdout().strip().split('\n'),
    install_dir: ASSETS_INSTALL_PATH
)

pkg.generate(
    kay,
    name: 'cz-kay',
    description: 'C++ GUI toolkit powered by Skia and Yoga.',
    version: meson.project_version(),
    filebase: 'cz-kay',
    subdirs: 'CZ/AK',
    requires: ['cz-skia'])
