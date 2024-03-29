import typing


__all__ = 'Expression', 'Print', 'Var', 'Block', 'If', 'While', 'Function', 'Return', 'Break', 'Class'

if typing.TYPE_CHECKING:
    from .base import Expr
    from .token import Token
    from .base import StmtVisitor as Visitor
    from .base import Stmt
    from .expr_ast import Variable
else:
    Expr = None
    Token = None
    Visitor = None
    Stmt = None
    Variable = None


class Expression:
    def __init__(self: 'typing.Self', expression: 'Expr') -> None:
        self.expression = expression

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_expression(self)


class Print:
    def __init__(self: 'typing.Self', expression: 'Expr') -> None:
        self.expression = expression

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_print(self)


class Var:
    def __init__(self: 'typing.Self', name: 'Token', expression: 'Expr') -> None:
        self.name = name
        self.expression = expression

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_var(self)


class Block:
    def __init__(self: 'typing.Self', statements: 'list[Stmt]') -> None:
        self.statements = statements

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_block(self)


class If:
    def __init__(self: 'typing.Self', condition: 'Expr', then_: 'Stmt', else_: 'Stmt|None') -> None:
        self.condition = condition
        self.then_ = then_
        self.else_ = else_

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_if(self)


class While:
    def __init__(self: 'typing.Self', condition: 'Expr', body: 'Stmt') -> None:
        self.condition = condition
        self.body = body

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_while(self)


class Function:
    def __init__(self: 'typing.Self', name: 'Token', params: 'list[Token]', body: 'Block') -> None:
        self.name = name
        self.params = params
        self.body = body

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_function(self)


class Return:
    def __init__(self: 'typing.Self', keyword: 'Token', value: 'Expr') -> None:
        self.keyword = keyword
        self.value = value

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_return(self)


class Break:
    def __init__(self: 'typing.Self', keyword: 'Token') -> None:
        self.keyword = keyword

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_break(self)


class Class:
    def __init__(self: 'typing.Self', name: 'Token', base: 'Variable|None', functions: 'list[Function]') -> None:
        self.name = name
        self.base = base
        self.functions = functions

    def accept(self: 'typing.Self', visitor: 'Visitor'):
        return visitor.visit_class(self)
