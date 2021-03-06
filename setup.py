from distutils.core import setup, Extension

module1 = Extension('gles',
            sources = ['glesmodule.c','gles_util.c'],
            libraries = ['GLES_CM']
            )

setup (name = 'gles',
    version = '0.1',
    description = 'OpenGL ES module',
    ext_modules = [module1]
    )
