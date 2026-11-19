#include "task.h"
#include "logged_command_wrapper.h"

/* Курсор влево */
class MoveCursorLeftCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        if (buffer.size() == 0) return;
        if (cursorPosition == 0) { 
            return;
        }
        else if (buffer[cursorPosition - 1] == '\n' && cursorPosition >= 2) {
            cursorPosition -= 2;
        }
        else {
            cursorPosition -=1;
        }
        editor.UnselectText();
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveCursorLeftCommand(*this);
    }
};

/* Курсор вправо */
class MoveCursorRightCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        if (cursorPosition == buffer.size()) {
            return;
        }
        else if (buffer[cursorPosition + 1] == '\n' && cursorPosition <= buffer.size() - 2) {
            cursorPosition += 2;
        }
        else {
            cursorPosition += 1;
        }
        editor.UnselectText();
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveCursorRightCommand(*this);
    }
};

/* Курсор вверх */
class MoveCursorUpCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        size_t end = buffer.rfind('\n', cursorPosition-1);
        if (end == std::string::npos) {
            return ;
        }
        size_t start = buffer.rfind('\n', end - 1);
        if (start == std::string::npos) {
            start = 0;
        } else {
            start += 1;
        }
        size_t delta_here = cursorPosition - end;
        size_t new_pos;
        if (end - start == 0) { ///?????????????????
            new_pos = start;
        } else if (delta_here > end - start + 1) {
            new_pos = end-1;
        } else {
            new_pos = start + delta_here - 1;
        }
        cursorPosition = new_pos;
        editor.UnselectText();
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveCursorUpCommand(*this);
    }
};

/* Курсор вниз */
class MoveCursorDownCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        size_t end = buffer.find('\n', cursorPosition);
        if (end == std::string::npos) {
            return ;
        }
        if (end+1 >= buffer.size()) {
            cursorPosition = buffer.size();
            return ;
        }
        size_t end_next = buffer.find('\n', end+1);
        if (end_next == std::string::npos) {
            end_next = buffer.size();
        }
        size_t start = buffer.rfind('\n', end-1);
        if (start == std::string::npos) {
            start = 0;
        } else {
            start += 1;
        }
        size_t delta_here = cursorPosition - start;
        size_t new_pos;
        if (delta_here > end_next - end+1) {
            new_pos = end_next;
        } else {
            new_pos = end+1 + delta_here;
        }
        cursorPosition = new_pos;
        editor.UnselectText();
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveCursorDownCommand(*this);
    }
};

/* Выделить текст */
class SelectTextCommand : public ICommand {
public:
    SelectTextCommand(size_t end) : end_(end) {}
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        editor.SelectText(cursorPosition, std::min(end_ + cursorPosition, buffer.size()));
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitSelectCommand(*this);
    }
private:
    size_t end_;
};

/* Ввести текст */
class InsertTextCommand : public ICommand {
public:
    InsertTextCommand(std::string text) : text_(text) {}
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        if (!editor.HasSelection()) {
            buffer.insert(cursorPosition, text_);
            cursorPosition += (text_.size());
        } else {
            auto [start, end]= editor.GetSelection();
            buffer.replace(start, end - start, text_);
            cursorPosition += text_.size();
            editor.UnselectText();
        }
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitInsertTextCommand(*this);
    }
private:
    std::string text_;
};

/* Удалить текст */
class DeleteTextCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        if (!editor.HasSelection()) {
            //??????????
            buffer.erase(cursorPosition, 1);
        } else {
            auto [start, end]= editor.GetSelection();
            buffer.erase(start, end - start);
            editor.UnselectText();
        }

        if (cursorPosition > buffer.size()) cursorPosition = buffer.size(); ////???
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitDeleteTextCommand(*this);
    }
};

/* Скопировать текст */
class CopyTextCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        if (!editor.HasSelection()) {
            clipboard = buffer.substr(cursorPosition, cursorPosition + 1); ///??????
        } else {
            auto [start, end]= editor.GetSelection();
            clipboard = buffer.substr(start, end - start);
            editor.UnselectText();
        }
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitCopyTextCommand(*this);
    }
};

/* Вставить скопированный текст */
class PasteTextCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        if (!editor.HasSelection()) {
            buffer.insert(cursorPosition, clipboard);
            cursorPosition += (clipboard.size());
        } else {
            auto [start, end]= editor.GetSelection();
            buffer.replace(start, end - start, clipboard);
            cursorPosition += clipboard.size();
            editor.UnselectText();
        }
    }
    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitPasteTextCommand(*this);
    }
};

/* Привести выделенный текст в ВЕРХНИЙ регистр */
class UppercaseTextCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        if (!editor.HasSelection()) {
            return;
        }
        auto [start, end]= editor.GetSelection();
        for (size_t k = start; k < end; ++k) {
            if (buffer[k] >= 'a' && buffer[k] <= 'z') {
                buffer[k] = buffer[k] - ('a' - 'A');
            }
        }
        editor.UnselectText();
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitUppercaseTextCommand(*this);
    }
};


class LowercaseTextCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        if (!editor.HasSelection()) {
            return;
        }
        auto [start, end] = editor.GetSelection();
        for (size_t k = start; k < end; ++k) {
            if (buffer[k] >= 'A' && buffer[k] <= 'Z') { 
                buffer[k] = buffer[k] + ('a' - 'A');
            }
        }
        editor.UnselectText();
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitLowercaseTextCommand(*this);
    }
};

/* Перенести курсор в конец строки */
class MoveToEndCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        size_t end = buffer.find('\n', cursorPosition);
        if (end == std::string::npos) {
            end = buffer.size();
        }
        cursorPosition = end;
        editor.UnselectText(); //??
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveToEndCommand(*this);
    }
};

/* Перенести курсор в начало строки */
class MoveToStartCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        size_t end;
        if (buffer[cursorPosition] == '\n') {
            end = buffer.rfind('\n', cursorPosition-1);
        } else {
            end = buffer.rfind('\n', cursorPosition);
        }
        if (end == std::string::npos) {
            end = 0;
        } else {
            end += 1;
        }
        cursorPosition = end;
        editor.UnselectText();
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitMoveToStartCommand(*this);
    }
};

/* Удалить часть строки, начиная от позиции курсора до первого пробела или конца строки */
class DeleteWordCommand : public ICommand {
public:
    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        size_t probel = buffer.find(' ', cursorPosition);
        size_t end = buffer.find('\n', cursorPosition);
        size_t word_end;
        if (end != std::string::npos && probel != std::string::npos && probel > end) {
            word_end = end;
            buffer.erase(cursorPosition, word_end - cursorPosition);
        } else if (end != std::string::npos && probel != std::string::npos && probel < end) {
            word_end = probel;
            buffer.erase(cursorPosition, word_end - cursorPosition);
        } else if (end != std::string::npos && probel == std::string::npos) {
            word_end = end;
            buffer.erase(cursorPosition, word_end - cursorPosition);
        } else if (end == std::string::npos && probel != std::string::npos) {
            word_end = probel;
            buffer.erase(cursorPosition, word_end - cursorPosition);
        } else {
            word_end = buffer.size();
            buffer.erase(cursorPosition, word_end - cursorPosition);
        }
        editor.UnselectText();
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        visitor.VisitDeleteWordCommand(*this);
    }
};

/* Макрос */
class MacroCommand : public ICommand {
public:
    MacroCommand(std::list<CommandPtr>& commands) : commands_(commands) {}

    void Apply(std::string& buffer, size_t& cursorPosition, std::string& clipboard, TextEditor& editor) override {
        for (auto& elem : commands_) {
            elem->Apply(buffer, cursorPosition, clipboard, editor);
        }
    }

    void AcceptVisitor(CommandVisitor& visitor) override {
        for (auto& elem : commands_) {
            elem->AcceptVisitor(visitor);
        }
    }
private:
    std::list<CommandPtr> commands_;
};


CommandBuilder::CommandBuilder() {}
CommandBuilder& CommandBuilder::WithType(Type type) {
    type_ = type;
    return *this;
}
CommandBuilder& CommandBuilder::SelectionSize(size_t selectionSize) {
    selectionSize_ = selectionSize;
    return *this;
}
CommandBuilder& CommandBuilder::Text(std::string text) {
    text_ = text;
    return *this;
}
CommandBuilder& CommandBuilder::LogTo(std::ostream& logStream) {
    logStreamPtr_ = &logStream;
    return *this;
}
CommandBuilder& CommandBuilder::AddSubcommand(CommandPtr subcommand) {
    subcommands_.push_back(subcommand);
    return *this;
}

CommandPtr CommandBuilder::build() {
    CommandPtr command;

    switch (type_) {
        case Type::MoveCursorLeft:
            command = std::make_shared<MoveCursorLeftCommand>();
            break;
        case Type::MoveCursorRight:
            command = std::make_shared<MoveCursorRightCommand>();
            break;
        case Type::MoveCursorUp:
            command = std::make_shared<MoveCursorUpCommand>();
            break;
        case Type::MoveCursorDown:
            command = std::make_shared<MoveCursorDownCommand>();
            break;
        case Type::SelectText:
            command = std::make_shared<SelectTextCommand>(selectionSize_);
            break;
        case Type::InsertText:
            command = std::make_shared<InsertTextCommand>(text_);
            break;
        case Type::DeleteText:
            command = std::make_shared<DeleteTextCommand>();
            break;
        case Type::CopyText:
            command = std::make_shared<CopyTextCommand>();
            break;
        case Type::PasteText:
            command = std::make_shared<PasteTextCommand>();
            break;
        case Type::UppercaseText:
            command = std::make_shared<UppercaseTextCommand>();
            break;
        case Type::LowercaseText:
            command = std::make_shared<LowercaseTextCommand>();
            break;
        case Type::MoveToEnd:
            command = std::make_shared<MoveToEndCommand>();
            break;
        case Type::MoveToStart:
            command = std::make_shared<MoveToStartCommand>();
            break;
        case Type::DeleteWord:
            command = std::make_shared<DeleteWordCommand>();
            break;
        case Type::Macro:
            command = std::make_shared<MacroCommand>(subcommands_);
            break;
        
    }
    if (logStreamPtr_) {
        command = std::make_shared<LoggedCommandWrapper>(*logStreamPtr_, command);
    }
    return command;
}

CommandLoggerVisitor::CommandLoggerVisitor(std::ostream& logStream) : logStream_(logStream) {}

void CommandLoggerVisitor::VisitMoveCursorLeftCommand(MoveCursorLeftCommand& command) {
    logStream_ << "h";
}

void CommandLoggerVisitor::VisitMoveCursorRightCommand(MoveCursorRightCommand& command) {
    logStream_ << "l";
}

void CommandLoggerVisitor::VisitMoveCursorUpCommand(MoveCursorUpCommand& command) {
    logStream_ << "k";
}

void CommandLoggerVisitor::VisitMoveCursorDownCommand(MoveCursorDownCommand& command) {
    logStream_ << "j";
}

void CommandLoggerVisitor::VisitSelectCommand(SelectTextCommand& command) {
    logStream_ << "v";
}

void CommandLoggerVisitor::VisitInsertTextCommand(InsertTextCommand& command) {
    logStream_ << "i";
}

void CommandLoggerVisitor::VisitDeleteTextCommand(DeleteTextCommand& command) {
    logStream_ << "d";
}

void CommandLoggerVisitor::VisitCopyTextCommand(CopyTextCommand& command) {
    logStream_ << "y";
}

void CommandLoggerVisitor::VisitPasteTextCommand(PasteTextCommand& command) {
    logStream_ << "p";
}

void CommandLoggerVisitor::VisitUppercaseTextCommand(UppercaseTextCommand& command) {
    logStream_ << "U";
}

void CommandLoggerVisitor::VisitLowercaseTextCommand(LowercaseTextCommand& command) {
    logStream_ << "u";
}

void CommandLoggerVisitor::VisitMoveToEndCommand(MoveToEndCommand& command) {
    logStream_ << "$";
}

void CommandLoggerVisitor::VisitMoveToStartCommand(MoveToStartCommand& command) {
    logStream_ << "0";
}

void CommandLoggerVisitor::VisitDeleteWordCommand(DeleteWordCommand& command) {
    logStream_ << "dE";
}