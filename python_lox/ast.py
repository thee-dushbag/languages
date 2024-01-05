import typing


__all__ = 'Binary', 'Grouping', 'Literal', 'Unary', 'Ternary'

if typing.TYPE_CHECKING:
    from .token import Token
    from .base import Expr
    from .base import Visitor
else:
    Token = None
    Expr = None
    Visitor = None


class Binary:
    def __init__(self: 'typing.Self', left: 'Expr', operator: 'Token', right: 'Expr') -> None:
        self.left = left
        self.operator = operator
        self.right = right

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_binary(self)


class Grouping:
    def __init__(self: 'typing.Self', expression: 'Expr') -> None:
        self.expression = expression

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_grouping(self)


class Literal:
    def __init__(self: 'typing.Self', value: 'object') -> None:
        self.value = value

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_literal(self)


class Unary:
    def __init__(self: 'typing.Self', operator: 'Token', right: 'Expr') -> None:
        self.operator = operator
        self.right = right

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_unary(self)


class Ternary:
    def __init__(self: 'typing.Self', condition: 'Expr', ontrue: 'Expr', onfalse: 'Expr') -> None:
        self.condition = condition
        self.ontrue = ontrue
        self.onfalse = onfalse

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_ternary(self)
