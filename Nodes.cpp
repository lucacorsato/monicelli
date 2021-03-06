/*
 * Monicelli: as esoteric language compiler
 * 
 * Copyright (C) 2014 Stefano Sanfilippo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Nodes.hpp"

using namespace monicelli;

static const std::string BLOCK = "    ";

std::ostream& monicelli::operator<<(std::ostream &stream, const Type &type) {
    switch (type) {
        case Type::INT:
            stream << "int";
            break;
        case Type::CHAR:
            stream << "char";
            break;
        case Type::FLOAT:
            stream << "float";
            break;
        case Type::BOOL:
            stream << "bool";
            break;
        case Type::DOUBLE:
            stream << "double";
            break;
    }

    return stream;
}

void emitIndent(std::ostream &stream, int indent) {
    for (int i = 0; i < indent; ++i) {
        stream << BLOCK;
    }
}

void Id::emit(std::ostream &stream, int indent) {
    stream << *value;
}

void Integer::emit(std::ostream &stream, int indent) {
    stream << value;
}

void Float::emit(std::ostream &stream, int indent) {
    stream << value;
}

void ExpNode::emit(std::ostream &stream, int indent) {
    left->emit(stream);
    stream << ' ' << getSym() << ' ';
    right->emit(stream);
}

void SemiExpNode::emit(std::ostream &stream, int indent) {
    SimpleExpression *e = dynamic_cast<SimpleExpression*>(left.get());
    bool braces = (e == nullptr);

    stream << ' ' << getSym() << ' ';
    if (braces) stream << "(";    
    left->emit(stream);
    if (braces) stream << ")";
}

void StatementList::emit(std::ostream &stream, int indent) {
    for (Statement *s: *this) {
        emitIndent(stream, indent);
        s->emit(stream, indent);
        stream << ";\n";
    }
}

void Return::emit(std::ostream &stream, int indent) {
    stream << "return";

    if (expression != nullptr) {
        stream << ' ';
        expression->emit(stream);
    }
}

void Loop::emit(std::ostream &stream, int indent) {
    stream << "do {\n";

    body->emit(stream, indent + 1);

    emitIndent(stream, indent);
    stream << "} while (";
    condition->emit(stream);
    stream << ")";
}

void BranchCase::emit(std::ostream &stream, int indent) {
    condition->emit(stream);
    stream << ") {\n";
    body->emit(stream, indent);
    emitIndent(stream, indent - 1);
    stream << "}";
}

void Branch::emit(std::ostream &stream, int indent) {
    stream << "if (";
    var->emit(stream);

    if (cases->size() > 0) {
        BranchCase *last = cases->back();
        for (BranchCase *c: *cases) {
            c->emit(stream, indent + 1);
            if (c != last) {
                stream << " else if (";
                var->emit(stream);
            }
        }
    }

    if (els == nullptr) {
        return;
    }

    stream << " else {\n";
    els->emit(stream, indent + 1);
    emitIndent(stream, indent);
    stream << "}";
}

void VarDeclaration::emit(std::ostream &stream, int indent) {
    stream << type << ' ';
    if (point) stream << '*';
    name->emit(stream);

    if (init != nullptr) {
        stream << " = ";
        init->emit(stream);
    }
}

void Assignment::emit(std::ostream &stream, int indent) {
    name->emit(stream);
    stream << " = ";
    value->emit(stream);
}

void Print::emit(std::ostream &stream, int indent) {
    stream << "std::cout << ";
    expression->emit(stream);
    stream << " << std::endl";
}

void Input::emit(std::ostream &stream, int indent) {
    stream << "std::cin >> ";
    variable->emit(stream);
}

void Abort::emit(std::ostream &stream, int indent) {
    stream << "std::exit(1)";
}

void Assert::emit(std::ostream &stream, int indent) {
    stream << "std::assert(";
    expression->emit(stream);
    stream << ")";
}

void FunctionCall::emit(std::ostream &stream, int indent) {
    name->emit(stream);
    stream << "(";
    args->emit(stream);
    stream << ")";
}

void Function::emit(std::ostream &stream, int indent) {
    emitIndent(stream, indent);

    stream << "void ";
    name->emit(stream);
    stream << "(";
    args->emit(stream);
    stream << ") {\n";
    body->emit(stream, indent + 1);
    stream << "}\n\n";
}

void Main::emit(std::ostream &stream, int indent) {
    emitIndent(stream, indent);

    stream << "int main() {\n";
    body->emit(stream, indent + 1);
    stream << "}\n";
}

void Program::emit(std::ostream &stream, int indent) {
    stream << "#include <iostream>\n";
    stream << "#include <cstdlib>\n\n";

    for (Function *f: functions) {
        f->emit(stream);
    }

    if (main != nullptr) {
        main->emit(stream);
    }
}

