<template>
  <div class="order-list-container">
    <h1>我的订单</h1>
    <div class="orders-table">
      <table>
        <thead>
          <tr>
            <th>订单号</th>
            <th>服务类型</th>
            <th>状态</th>
            <th>创建时间</th>
            <th>操作</th>
          </tr>
        </thead>
        <tbody>
          <tr
            v-for="order in orders"
            :key="order.id"
          >
            <td>{{ order.id }}</td>
            <td>{{ order.serviceType }}</td>
            <td>
              <span :class="['status', order.status]">{{ order.statusText }}</span>
            </td>
            <td>{{ order.createdAt }}</td>
            <td>
              <button
                v-if="order.status === 'in_progress'"
                class="track-button"
                @click="trackOrder(order.id)"
              >
                实时追踪
              </button>
              <button
                v-else
                class="details-button"
                @click="viewDetails(order.id)"
              >
                查看详情
              </button>
            </td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<script>
export default {
  name: 'OrderList',
  data() {
    return {
      orders: [
        {
          id: 'TSK001',
          serviceType: '物流配送',
          status: 'in_progress',
          statusText: '进行中',
          createdAt: '2023-01-15 14:30'
        },
        {
          id: 'TSK002',
          serviceType: '农业植保',
          status: 'completed',
          statusText: '已完成',
          createdAt: '2023-01-14 09:15'
        },
        {
          id: 'TSK003',
          serviceType: '巡检服务',
          status: 'pending',
          statusText: '待处理',
          createdAt: '2023-01-16 10:00'
        }
      ]
    }
  },
  methods: {
    trackOrder(orderId) {
      // 跳转到实时任务追踪页面
      this.$router.push(`/task-tracking/${orderId}`)
    },
    viewDetails(orderId) {
      // 查看订单详情（这里可以跳转到详情页）
      console.log('View details for order:', orderId)
    }
  }
}
</script>

<style scoped>
.order-list-container {
  max-width: 1200px;
  margin: 0 auto;
  padding: 2rem;
}

h1 {
  text-align: center;
  color: #333;
  margin-bottom: 2rem;
}

.orders-table {
  background: white;
  border-radius: 8px;
  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
  overflow: hidden;
}

table {
  width: 100%;
  border-collapse: collapse;
}

th,
td {
  padding: 1rem;
  text-align: left;
  border-bottom: 1px solid #eee;
}

th {
  background-color: #f8f9fa;
  font-weight: 600;
  color: #555;
}

.status {
  padding: 0.25rem 0.5rem;
  border-radius: 4px;
  font-size: 0.9rem;
  font-weight: 500;
}

.status.in_progress {
  background-color: #409eff;
  color: white;
}

.status.completed {
  background-color: #67c23a;
  color: white;
}

.status.pending {
  background-color: #e6a23c;
  color: white;
}

.track-button,
.details-button {
  padding: 0.5rem 1rem;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 0.9rem;
  transition: background-color 0.3s;
}

.track-button {
  background-color: #409eff;
  color: white;
}

.track-button:hover {
  background-color: #66b1ff;
}

.details-button {
  background-color: #f0f2f5;
  color: #666;
}

.details-button:hover {
  background-color: #e0e3e9;
}
</style>