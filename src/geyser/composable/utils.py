from tempfile import TemporaryDirectory
from os import makedirs, environ
from pathlib import Path

from geyser import Geyser


@Geyser.composable()
class PathManager:
    _tempdir = TemporaryDirectory(suffix='_folder', prefix='geyser_')
    _homedir = Path.home().absolute()
    _curdir = Path('.').absolute()

    @classmethod
    def _makedirs_join(cls, *args, root_dir) -> Path:
        new_path = root_dir.joinpath(*args)
        try:
            makedirs(new_path.parent)
        except FileExistsError:
            pass
        return new_path

    def temporary(self, *args) -> Path:
        root_dir = Path(self._tempdir.name).absolute()
        return self._makedirs_join(*args, root_dir=root_dir)

    def home(self, *args) -> Path:
        root_dir = self._homedir.joinpath('geyser')
        return self._makedirs_join(*args, root_dir=root_dir)

    def current(self, *args) -> Path:
        root_dir = self._curdir / 'geyser'
        return self._makedirs_join(*args, root_dir=root_dir)


@Geyser.composable(auto_compose=False)
class EnvManager:
    def __init__(self, *args, **kwargs):
        if 'logger' in kwargs:
            self.logger = kwargs['logger']
            kwargs.pop('logger')

        for key, item in kwargs.items():
            environ[key] = item

    def __getitem__(self, item):
        self.logger.debug(f'Get "{item}" from environ.')
        return environ.__getitem__(item)

    def __setitem__(self, key, value):
        self.logger.debug(f'Set "{key}" in environ to "{value}".')
        return environ.__setitem__(key, value)

    def __delitem__(self, key):
        self.logger.debug(f'Remove "{key}" in environ.')
        return environ.__delitem__(key)


@Geyser.executable()
def runtime_info(ctx):
    import platform

    ctx.logger.info(f'Geyser v{Geyser.version()}')
    ctx.logger.info(f'Geyser Core build {Geyser.core_build()}')
    ctx.logger.info(f'Python build {platform.python_compiler()} {platform.python_build()[1]}')
    ctx.logger.info(f'Operating System {platform.platform()}')


@Geyser.composable(auto_compose=False)
class ObjectPool(dict):
    def __getattr__(self, item):
        if item in self.__dir__():
            return getattr(super(ObjectPool, self), item)
        else:
            return self[item]

    def __setattr__(self, key, value):
        if key in self.__dir__():
            return setattr(super(ObjectPool, self), key, value)
        else:
            self[key] = value
