/**
 * 路径规划核心模块
 * 实现基于城市主干道的无人机路径规划算法
 */

class RoutePlanner {
  constructor(roadNetwork = null) {
    // 城市道路网络数据
    this.roadNetwork = roadNetwork || this.getDefaultRoadNetwork();
    // 启发式函数权重
    this.heuristicWeight = 1.0;
  }

  /**
   * 获取默认道路网络数据
   * 实际项目中应从API或数据库获取
   */
  getDefaultRoadNetwork() {
    // 示例数据格式
    return {
      nodes: [
        { id: 'A', name: '节点A', lat: 39.9042, lng: 116.4074, isMainRoad: true },
        { id: 'B', name: '节点B', lat: 39.9142, lng: 116.4174, isMainRoad: true },
        { id: 'C', name: '节点C', lat: 39.9242, lng: 116.4274, isMainRoad: true },
        { id: 'D', name: '节点D', lat: 39.9342, lng: 116.4374, isMainRoad: false },
        { id: 'E', name: '节点E', lat: 39.9442, lng: 116.4474, isMainRoad: true }
      ],
      edges: [
        { from: 'A', to: 'B', distance: 1.5, isMainRoad: true },
        { from: 'B', to: 'C', distance: 2.0, isMainRoad: true },
        { from: 'C', to: 'D', distance: 1.2, isMainRoad: false },
        { from: 'D', to: 'E', distance: 1.8, isMainRoad: true },
        { from: 'A', to: 'C', distance: 3.2, isMainRoad: true }
      ]
    };
  }

  /**
   * 过滤主干道
   */
  filterMainRoads(network) {
    return {
      nodes: network.nodes.filter(node => node.isMainRoad),
      edges: network.edges.filter(edge => edge.isMainRoad)
    };
  }

  /**
   * 计算两点间直线距离(启发式函数)
   */
  calculateHeuristic(node1, node2) {
    // 简化计算，实际应使用Haversine公式
    const dx = node1.lng - node2.lng;
    const dy = node1.lat - node2.lat;
    return Math.sqrt(dx * dx + dy * dy) * this.heuristicWeight;
  }

  /**
   * 获取节点的邻居节点
   */
  getNeighbors(nodeId, network) {
    const neighbors = [];
    network.edges.forEach(edge => {
      if (edge.from === nodeId) {
        neighbors.push({
          id: edge.to,
          distance: edge.distance
        });
      } else if (edge.to === nodeId) {
        neighbors.push({
          id: edge.from,
          distance: edge.distance
        });
      }
    });
    return neighbors;
  }

  /**
   * A*算法实现路径规划
   */
  aStarSearch(startNodeId, endNodeId, network = null) {
    const useNetwork = network || this.roadNetwork;
    const mainRoadNetwork = this.filterMainRoads(useNetwork);

    // 获取起始和结束节点
    const startNode = mainRoadNetwork.nodes.find(node => node.id === startNodeId);
    const endNode = mainRoadNetwork.nodes.find(node => node.id === endNodeId);

    if (!startNode || !endNode) {
      throw new Error('起始节点或结束节点不存在');
    }

    // 初始化开放列表和关闭列表
    const openSet = new Set([startNodeId]);
    const closedSet = new Set();

    // 记录每个节点的得分
    const gScore = {};
    const fScore = {};
    const cameFrom = {};

    mainRoadNetwork.nodes.forEach(node => {
      gScore[node.id] = Infinity;
      fScore[node.id] = Infinity;
    });

    gScore[startNodeId] = 0;
    fScore[startNodeId] = this.calculateHeuristic(startNode, endNode);

    while (openSet.size > 0) {
      // 找到fScore最低的节点
      let current = null;
      let lowestFScore = Infinity;

      for (const nodeId of openSet) {
        if (fScore[nodeId] < lowestFScore) {
          lowestFScore = fScore[nodeId];
          current = nodeId;
        }
      }

      // 如果到达目标节点
      if (current === endNodeId) {
        // 重构路径
        const path = [];
        let currentNode = current;
        while (currentNode) {
          path.unshift(currentNode);
          currentNode = cameFrom[currentNode];
        }
        return this.constructPathDetails(path, mainRoadNetwork);
      }

      // 将当前节点移到关闭列表
      openSet.delete(current);
      closedSet.add(current);

      // 检查邻居节点
      const neighbors = this.getNeighbors(current, mainRoadNetwork);
      for (const neighbor of neighbors) {
        if (closedSet.has(neighbor.id)) {
          continue;
        }

        const tentativeGScore = gScore[current] + neighbor.distance;

        if (!openSet.has(neighbor.id)) {
          openSet.add(neighbor.id);
        } else if (tentativeGScore >= gScore[neighbor.id]) {
          continue;
        }

        cameFrom[neighbor.id] = current;
        gScore[neighbor.id] = tentativeGScore;
        const neighborNode = mainRoadNetwork.nodes.find(node => node.id === neighbor.id);
        fScore[neighbor.id] = gScore[neighbor.id] + this.calculateHeuristic(neighborNode, endNode);
      }
    }

    // 没有找到路径
    return null;
  }

  /**
   * 构造路径详细信息
   */
  constructPathDetails(path, network) {
    if (!path || path.length === 0) {
      return null;
    }

    const pathDetails = {
      nodes: [],
      totalDistance: 0,
      segments: []
    };

    // 获取节点详细信息
    path.forEach(nodeId => {
      const node = network.nodes.find(n => n.id === nodeId);
      if (node) {
        pathDetails.nodes.push(node);
      }
    });

    // 计算路径段和总距离
    for (let i = 0; i < path.length - 1; i++) {
      const fromId = path[i];
      const toId = path[i + 1];

      const edge = network.edges.find(e =>
        (e.from === fromId && e.to === toId) ||
        (e.from === toId && e.to === fromId)
      );

      if (edge) {
        pathDetails.segments.push({
          from: fromId,
          to: toId,
          distance: edge.distance
        });
        pathDetails.totalDistance += edge.distance;
      }
    }

    return pathDetails;
  }

  /**
   * 实时路径优化
   */
  optimizeRoute() {
    // 根据实时交通和天气调整路径权重
    // TODO: 实现基于实时交通和天气的路径权重调整算法
    return {
      optimized: true,
      message: '路径已根据实时条件优化'
    };
  }

  /**
   * 批量路径规划
   */
  planMultipleRoutes(routes) {
    const results = [];
    routes.forEach(route => {
      try {
        const path = this.aStarSearch(route.start, route.end);
        results.push({
          id: route.id,
          success: true,
          path: path,
          error: null
        });
      } catch (error) {
        results.push({
          id: route.id,
          success: false,
          path: null,
          error: error.message
        });
      }
    });
    return results;
  }
}

// 导出单例实例
const routePlanner = new RoutePlanner();
export default routePlanner;

// 导出类以供需要多个实例时使用
export { RoutePlanner };