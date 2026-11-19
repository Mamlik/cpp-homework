#pragma once

#include "torrent_file.h"
#include "piece.h"
#include <queue>
#include <string>
#include <unordered_set>
#include <mutex>

/*
 * Хранилище информации о частях скачиваемого файла.
 * В этом классе отслеживается информация о том, какие части файла осталось скачать
 */
class PieceStorage {
public:
    explicit PieceStorage(const TorrentFile& tf) {
        for (size_t i = 0; i < tf.pieceHashes.size(); ++i) {
            size_t length = tf.pieceLength;
            if (i + 1 == tf.pieceHashes.size()) {
                size_t covered = tf.pieceLength * (tf.pieceHashes.size() - 1);
                length = tf.length - covered;
            }
            remainPieces_.emplace(std::make_shared<Piece>(i, length, tf.pieceHashes[i]));
        }
    }

    /*
     * Отдает указатель на следующую часть файла, которую надо скачать
     */
    PiecePtr GetNextPieceToDownload();

    /*
     * Эта функция вызывается из PeerConnect, когда скачивание одной части файла завершено.
     * В рамках данного задания требуется очистить очередь частей для скачивания как только хотя бы одна часть будет успешно скачана.
     */
    void PieceProcessed(const PiecePtr& piece);

    /*
     * Остались ли нескачанные части файла?
     */
    bool QueueIsEmpty() const;

    /*
     * Сколько частей файла всего
     */
    size_t TotalPiecesCount() const;

protected:
    std::queue<PiecePtr> remainPieces_;  // очередь частей файла, которые осталось скачать

    virtual void SavePieceToDisk(PiecePtr piece);
};
