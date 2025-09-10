#include "simple_bvh.h"
#include "spatial_partition.h"

namespace mite {

std::unique_ptr<SpatialPartition> CreateSpatialPartition(SpatialPartitionType type)
{
  switch (type) {
    case SpatialPartitionType::BVH:
      return std::make_unique<SimpleBVH>();
    case SpatialPartitionType::QuadTree:
    case SpatialPartitionType::Octree:
    case SpatialPartitionType::Grid:
    case SpatialPartitionType::KDTree:
      // 这些类型暂不实现，抛出异常
      throw std::runtime_error("Spatial partition type not implemented yet");
    default:
      throw std::runtime_error("Unknown spatial partition type");
  }
}

const char *GetSpatialPartitionTypeName(SpatialPartitionType type)
{
  switch (type) {
    case SpatialPartitionType::BVH:
      return "BVH";
    case SpatialPartitionType::QuadTree:
      return "QuadTree";
    case SpatialPartitionType::Octree:
      return "Octree";
    case SpatialPartitionType::Grid:
      return "Grid";
    case SpatialPartitionType::KDTree:
      return "KDTree";
    default:
      return "Unknown";
  }
}

}  // namespace mite
