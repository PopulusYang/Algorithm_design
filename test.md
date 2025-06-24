```mermaid
graph TD
    A[初始化dp_max和prev] --> B[遍历所有格子i,j]
    B --> C{是否障碍?}
    C -->|是| D[跳过]
    C -->|否| E[遍历四个相邻格子ni,nj]
    E --> F{ni,nj是否合法且非障碍?}
    F -->|否| G[继续检查其他方向]
    F -->|是| H[计算新权值: dp_max[ni][nj]+当前权值+资源影响]
    H --> I{新权值 > dp_max[i][j]?}
    I -->|是| J[更新dp_max和prev]
    I -->|否| G
    G --> K{是否检查完所有方向?}
    K -->|否| E
    K -->|是| L{是否遍历完所有格子?}
    L -->|否| B
    L -->|是| M[返回dp_max[end]和路径]
```