executable(
    'kay-wayland-client',
    sources : ['main.cpp',
        wl_mod.scan_xml(wl_mod.find_protocol('xdg-shell')),
        wl_mod.scan_xml(wl_mod.find_protocol('xdg-decoration', state : 'unstable', version: 1))],
    dependencies : [
        kay_dep,
        wayland_client_dep,
        wayland_egl_dep,
        egl_dep
    ],
    install : true)
