sources = run_command('find', '.', '-type', 'f', '-name', '*[.c,.cpp,.h,.hpp]', check : false).stdout().strip().split('\n')

executable(
    'basic',
    sources : sources,
    dependencies : [
        kay_dep
    ],
    install : false)

