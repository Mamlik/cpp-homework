from collections.abc import Callable, Hashable, Iterable
from collections import OrderedDict, namedtuple
from functools import wraps

def lru_cache(max_items: int) -> Callable:
    """
    Функция создает декоратор, позволяющий кэшировать результаты выполнения обернутой функции по принципу LRU-кэша.
    Размер LRU кэша ограничен количеством max_items. При попытке сохранить новый результат в кэш, в том случае, когда
    размер кэша уже равен max_size, происходит удаление одного из старых элементов, сохраненных в кэше.
    Удаляется тот элемент, к которому обращались давнее всего.
    Также у обернутой функции должен появиться атрибут stats, в котором лежит объект с атрибутами cache_hits и
    cache_misses, подсчитывающие количество успешных и неуспешных использований кэша.
    :param max_items: максимальный размер кэша.
    :return: декоратор, добавляющий LRU-кэширование для обернутой функции.
    """
    def decorator(f):
        cache = OrderedDict()
        hits = 0
        misses = 0
        @wraps(f)
        def wrapper(*args, **kwargs):
            nonlocal hits, misses
            key = repr((args, sorted(kwargs.items())))
            if key in cache:
                hits += 1
                cache.move_to_end(key)
                return cache[key]
            else:
                misses += 1
                res = f(*args, **kwargs)
                cache[key] = res
                if len(cache) > max_items:
                    cache.popitem(last=False)
                return res
        class StatsView:
            @property
            def cache_hits(self):
                return hits
            @property
            def cache_misses(self):
                return misses

        wrapper.stats = StatsView()
        return wrapper

    return decorator
