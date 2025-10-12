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
HEADERS_INSTALL_DIR = join_paths(get_option('prefix'), get_option('includedir'))
FONTS_INSTALL_DIR = join_paths('/usr/share/fonts/cuarzo')
ASSETS_INSTALL_DIR = join_paths(get_option('prefix'), get_option('datadir'), 'Kay/Assets')
CONF_FILE_NAME = 'CZ' + meson.project_name() + 'Version.h'

conf = configuration_data()
conf.set('VERSION_MAJOR', VERSION_MAJOR)
conf.set('VERSION_MINOR', VERSION_MINOR)
conf.set('VERSION_PATCH', VERSION_PATCH)
conf_file = configure_file(
    input : 'src/CZ/AK/' + CONF_FILE_NAME +'.in',
    output : CONF_FILE_NAME,
    configuration : conf)

install_data(
    conf_file,
    install_dir: HEADERS_INSTALL_DIR)

add_project_arguments([
    '-DCZ_KAY_ASSETS_DIR="@0@"'.format(ASSETS_INSTALL_DIR),
    '-DCZ_KAY_FONTS_DIR="@0@"'.format(FONTS_INSTALL_DIR)
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
    install_headers(header_files, install_dir : join_paths(HEADERS_INSTALL_DIR, header_dir[1]))
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
    install_dir: ASSETS_INSTALL_DIR
)

install_data(
    run_command('find', './assets/fonts', '-type', 'f', '-name', '*[.ttf,.otf,.codepoints]', check : false).stdout().strip().split('\n'),
    install_dir: FONTS_INSTALL_DIR
)

pkg.generate(
    kay,
    name: 'cz-kay',
    description: 'C++ GUI toolkit powered by Skia and Yoga.',
    version: meson.project_version(),
    filebase: 'cz-kay',
    subdirs: ['CZ', 'CZ/AK', 'CZ/AK/Nodes', 'CZ/AK/Events', 'CZ/AK/Effects', 'CZ/AK/Input'],
    libraries: deps)
