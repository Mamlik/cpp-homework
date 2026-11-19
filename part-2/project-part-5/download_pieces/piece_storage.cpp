#include "piece_storage.h"
#include <iostream>

PiecePtr PieceStorage::GetNextPieceToDownload() {
    if (remainPieces_.empty()) {
        return nullptr;
    }
    PiecePtr piece = remainPieces_.front();
    remainPieces_.pop();
    return piece;
}

void PieceStorage::PieceProcessed(const PiecePtr& piece) {
    std::queue<PiecePtr> emptyQueue;
    std::swap(remainPieces_, emptyQueue);
}

bool PieceStorage::QueueIsEmpty() const {
    return remainPieces_.empty();
}

size_t PieceStorage::TotalPiecesCount() const {
    return remainPieces_.size();
}

void PieceStorage::SavePieceToDisk(PiecePtr piece) {
    // Эта функция будет переопределена при запуске вашего решения в проверяющей системе
    // Вместо сохранения на диск там пока что будет проверка, что часть файла скачалась правильно
    std::cout << "Downloaded piece " << piece->GetIndex() << std::endl;
    std::cerr << "Clear pieces list, don't want to download all of them" << std::endl;
    while (!remainPieces_.empty()) {
        remainPieces_.pop();
    }
}
