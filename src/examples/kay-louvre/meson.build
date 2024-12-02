louvre_dep = dependency('Louvre')
egl_dep = dependency('egl')
pixmap_dep = dependency('pixman-1')

executable(
    'kay-louvre',
    sources : ['main.cpp'],
    dependencies : [
        kay_dep,
        louvre_dep,
        pixmap_dep,
        egl_dep
    ],
    install : true)
