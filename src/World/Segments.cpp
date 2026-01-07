#include "Segments.hpp"
#include "Terrain.hpp"

namespace Segments
{
    TerrainSegment generateNextSegment(Terrain &terrain)
    {
        TerrainSegment seg;

        seg.startX = terrain.nextSegmentX;

        // Type de segment
        std::uniform_int_distribution<int> dist(0, 3);
        seg.type = static_cast<SegmentType>(dist(terrain.rng));

        // sol sur 1 ou 2 hauteurs
        std::uniform_int_distribution<int> niv(1, GROUND_HEIGHT);
        int nbNiv = niv(terrain.rng) / TILE;

        const float groundY = Config::Get().windowSize.y - TILE * (nbNiv + 1);
        const int tilesX = static_cast<int>(SEGMENT_WIDTH / TILE);
        int tilesY = static_cast<int>(GROUND_HEIGHT / TILE);

        tilesY = nbNiv;

        auto groundVisual = [&](int x, int y)
        {
            if (y == 0)
            {
                if (x == 0)
                    return BlockVisual::GroundTopLeft;
                if (x == tilesX - 1)
                    return BlockVisual::GroundTopRight;
                return BlockVisual::GroundTopMid;
            }
            else
            {
                if (x == 0)
                    return BlockVisual::GroundFillLeft;
                if (x == tilesX - 1)
                    return BlockVisual::GroundFillRight;
                return BlockVisual::GroundFillMid;
            }
        };

        auto ceilingVisual = [&](int x)
        {
            if (x == 0)
                return BlockVisual::CeilingLeft;
            if (x == tilesX - 1)
                return BlockVisual::CeilingRight;
            return BlockVisual::CeilingMid;
        };

        // ------------------------
        // SOL / SEGMENT NORMAL
        // ------------------------
        switch (seg.type)
        {
        case SegmentType::Flat:
        case SegmentType::Corridor:
        case SegmentType::TurretZone:
        {

            // Sol
            for (int y = 0; y <= tilesY; ++y)
            {
                std::uniform_int_distribution<int> dist(1, tilesX - 2);
                int turretBlock = dist(terrain.rng);

                for (int x = 0; x < tilesX; ++x)
                {
                    BlockVisual v = groundVisual(x, y);

                    bool isSurface =
                        (y == 0) && // uniquement la surface
                        (v == BlockVisual::GroundTopMid);

                    bool hasTurret = (seg.type == SegmentType::TurretZone && turretBlock == x && isSurface);

                    seg.blocks.push_back({sf::FloatRect{{x * TILE, groundY + y * TILE}, {TILE, TILE}},
                                          v,
                                          hasTurret});
                }
            }

            // Plafond uniquement pour Corridor
            if (seg.type == SegmentType::Corridor)
            {
                for (int x = 0; x < tilesX; ++x)
                {
                    seg.blocks.push_back({sf::FloatRect{{x * TILE, 0.f}, {TILE, TILE}}, ceilingVisual(x), false});
                }
            }
            break;
        }

        case SegmentType::Hole:
        {
            const float holeWidth = SEGMENT_WIDTH * 0.4f;
            const float sideWidth = (SEGMENT_WIDTH - holeWidth) * 0.5f;

            const int leftCount = static_cast<int>(sideWidth / TILE);
            const int rightCount = leftCount;

            // Plateforme gauche
            for (int i = 0; i < leftCount; ++i)
            {
                BlockVisual v = (i == 0)               ? BlockVisual::GroundTopLeft
                                : (i == leftCount - 1) ? BlockVisual::GroundTopRight
                                                       : BlockVisual::GroundTopMid;
                seg.blocks.push_back({sf::FloatRect{{i * TILE, groundY}, {TILE, TILE}}, v, false});
            }

            // Plateforme droite
            float rightStartX = sideWidth + holeWidth;
            for (int i = 0; i < rightCount; ++i)
            {
                BlockVisual v = (i == 0)                ? BlockVisual::GroundTopLeft
                                : (i == rightCount - 1) ? BlockVisual::GroundTopRight
                                                        : BlockVisual::GroundTopMid;
                seg.blocks.push_back({sf::FloatRect{{rightStartX + i * TILE, groundY}, {TILE, TILE}}, v, false});
            }
            break;
        }
        }

        // ------------------------
        // PASSERELLES CENTRALES ALÉATOIRES
        // ------------------------
        std::uniform_int_distribution<int> hasBridgeDist(0, 2); // moins fréquent : 1 chance sur 3
        if (hasBridgeDist(terrain.rng) == 0 &&
            (seg.type == SegmentType::Flat || seg.type == SegmentType::Corridor || seg.type == SegmentType::TurretZone))
        {
            // Hauteur aléatoire au-dessus du sol
            std::uniform_int_distribution<int> heightDist(150, 300);
            float bridgeY = groundY - static_cast<float>(heightDist(terrain.rng));

            int bridgeTiles = tilesX / 3 + 1; // plus court que le segment
            int startXTile = (tilesX - bridgeTiles) / 2;

            for (int i = 0; i < bridgeTiles; ++i)
            {
                BlockVisual v;
                if (i == 0)
                    v = BlockVisual::BridgeLeft;
                else if (i == bridgeTiles - 1)
                    v = BlockVisual::BridgeRight;
                else
                    v = BlockVisual::BridgeMid;

                TerrainBlock blk;
                blk.rect = sf::FloatRect{{(startXTile + i) * TILE, bridgeY}, {TILE, TILE}};
                blk.visual = v;
                blk.hasTurret = (blk.visual == BlockVisual::BridgeMid); // marquer pour futur placement de tourelle
                seg.blocks.push_back(blk);
            }
        }

        // ------------------------
        // PASSERELLES PLAFOND (Corridor ou TurretZone)
        // ------------------------
        if (seg.type == SegmentType::Corridor || seg.type == SegmentType::TurretZone)
        {
            std::uniform_int_distribution<int> ceilingBridgeDist(0, 3);
            if (ceilingBridgeDist(terrain.rng) == 0)
            {
                int bridgeTiles = tilesX / 3;
                int startXTile = (tilesX - bridgeTiles) / 2;
                float ceilingY = 0.f;

                for (int i = 0; i < bridgeTiles; ++i)
                {
                    BlockVisual v;
                    if (i == 0)
                        v = BlockVisual::CeilingLeft;
                    else if (i == bridgeTiles - 1)
                        v = BlockVisual::CeilingRight;
                    else
                        v = BlockVisual::CeilingMid;

                    TerrainBlock blk;
                    blk.rect = sf::FloatRect{{(startXTile + i) * TILE, ceilingY}, {TILE, TILE}};
                    blk.visual = v;
                    blk.hasTurret = (blk.visual == BlockVisual::CeilingMid); // possibilité de tourelle plafond
                    seg.blocks.push_back(blk);
                }
            }
        }

        terrain.segments.push_back(seg);
        terrain.nextSegmentX += SEGMENT_WIDTH - TILE;

        return seg;
    }
}
