class EnhancedList(list):
    """
    Улучшенный list.
    Данный класс является наследником класса list и добавляет к нему несколько новых атрибутов.

    - first -- позволяет получать и задавать значение первого элемента списка.
    - last -- позволяет получать и задавать значение последнего элемента списка.
    - size -- позволяет получать и задавать длину списка:
        - если новая длина больше старой, то дополнить список значениями None;
        - если новая длина меньше старой, то удалять значения из конца списка.
    """
    def __init__(self, *args):
        super().__init__(args)

    def __init__(self, iterable=()):
        super().__init__(iterable)
    
    @property
    def first(self):
        return self[0] if self else None
    
    @first.setter
    def first(self, value):
        if self:
            self[0] = value
        else:
            raise IndexError("Cannot set first element of an empty list.")

    @property
    def last(self):
        return self[-1] if self else None
    
    @last.setter
    def last(self, value):
        if self:
            self[-1] = value
        else:
            raise IndexError("Cannot set last element of an empty list.")
    
    @property
    def size(self):
        return len(self)
    
    @size.setter
    def size(self, value):
        if value > len(self):
            self.extend([None] * (value - len(self)))
        elif value < len(self):
            del self[value:]



