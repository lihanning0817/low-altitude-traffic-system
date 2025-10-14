<template>
  <v-container>
    <v-row>
      <v-col cols="12">
        <v-card>
          <v-card-title>飞行任务详情</v-card-title>
          <v-card-text>
            <v-list>
              <v-list-item>
                <v-list-item-content>
                  <v-list-item-title>任务名称: {{ task.name }}</v-list-item-title>
                  <v-list-item-subtitle>状态: {{ task.status }}</v-list-item-subtitle>
                  <v-list-item-subtitle>起点: {{ task.start }}</v-list-item-subtitle>
                  <v-list-item-subtitle>终点: {{ task.end }}</v-list-item-subtitle>
                  <v-list-item-subtitle>飞行器: {{ task.aircraft }}</v-list-item-subtitle>
                </v-list-item-content>
              </v-list-item>
            </v-list>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
export default {
  name: "FlightTaskDetail",
  data() {
    return {
      task: {
        name: '任务1',
        status: '进行中',
        start: '北京',
        end: '上海',
        aircraft: '飞行器1'
      }
    };
  },
  mounted() {
    // const taskId = this.$route.params.id;
    // TODO: 根据taskId加载任务详情
  }
};
</script>

<style scoped>
/* 样式根据需要进行调整 */
</style>
