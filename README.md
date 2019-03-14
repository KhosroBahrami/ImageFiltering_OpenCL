# Image Filtering using OpenCL




much faster than two steps RPN-based approaches. The following table compares SSD, Faster RCNN and YOLO.

| Object Detection Method | VOC2007 test mAP |  Speed (FPS) | Number of Prior Boxes | Input Resolution |
| :---: |   :---:     | :---: | :---: | :---: |
| Faster R-CNN (VGG16) | 73.2% | 7 | 6000 | 1000*600 |
| YOLOv1 (VGG16) | 63.4% | 45 |  98 | 448*448 |
| SSD300 (VGG16) | 74.3% | 59 | 8732 | 300*300 |
| SSD512 (VGG16) | 76.9% | 22 | 24564 | 512*512 |

### 



| Filtering Method  |  Speed on CPU (ms) | Speen on GPU (ms) | Image Resolution |
| :---: |   :---:   | :---: | :---: | :---: |
| Box Filter        | 3.26 | 0.93 | 800*600 |
| Gaussian Filter   | 3.17 | 0.95 | 800*600 |
| Laplacian Filter  | 3.55 | 0.92 | 800*600 |
| Sharpening Filter | 3.18 | 0.92 | 800*600 |



