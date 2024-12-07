louvre_dep = dependency('Louvre')
pixmap_dep = dependency('pixman-1')

executable(
    'kay-louvre',
    sources : ['main.cpp'],
    dependencies : [
        kay_dep,
        louvre_dep,
        pixmap_dep,
        glesv2_dep,
        egl_dep
    ],
    install : true)
