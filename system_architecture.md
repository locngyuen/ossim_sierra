# Kiến Trúc Hệ Thống Mô Phỏng Hệ Điều Hành

## Tổng Quan Hệ Thống

Đây là một hệ thống mô phỏng hệ điều hành (OS Simulator) được thiết kế để mô phỏng các thành phần cốt lõi của một hệ điều hành thực tế. Hệ thống này bao gồm các module quản lý bộ nhớ, lập lịch CPU, xử lý syscall và mô phỏng các tiến trình.

## Cấu Trúc Thư Mục

```
.
├── include/        # Các file header định nghĩa cấu trúc dữ liệu và API
├── src/            # Mã nguồn của các module
├── input/          # Dữ liệu đầu vào, bao gồm các tiến trình mẫu
├── output/         # Kết quả đầu ra của chương trình
└── Makefile        # Cấu hình biên dịch dự án
```

## Các Thành Phần Chính

### 1. Nhân hệ điều hành (OS Core)

Tệp `os.c` là trung tâm của hệ thống mô phỏng, quản lý việc khởi động, lập lịch và điều phối tất cả các tiến trình trong hệ thống.

- **Khởi tạo hệ thống**: Đọc cấu hình, khởi tạo bộ nhớ và CPU
- **Quản lý tiến trình**: Tạo và điều phối các tiến trình
- **Quản lý tài nguyên**: Phân bổ CPU và bộ nhớ

### 2. Quản Lý Bộ Nhớ (Memory Management)

Hệ thống quản lý bộ nhớ được thực hiện qua các module sau:

- **mm.c/mm.h**: Cài đặt cấu trúc quản lý bộ nhớ 
- **mm-vm.c**: Quản lý bộ nhớ ảo
- **mm-memphy.c**: Quản lý bộ nhớ vật lý
- **paging.c**: Cài đặt hệ thống phân trang
- **mem.c/mem.h**: Các thao tác cấp phát và giải phóng bộ nhớ

Khi một tiến trình được tạo, nó được cấp phát không gian bộ nhớ ảo. Cơ chế phân trang chuyển đổi địa chỉ ảo sang địa chỉ vật lý.

### 3. Quản Lý CPU và Lập Lịch (CPU & Scheduling)

- **cpu.c/cpu.h**: Mô phỏng hoạt động của CPU
- **sched.c/sched.h**: Cài đặt thuật toán lập lịch tiến trình
- **timer.c/timer.h**: Quản lý thời gian và ngắt timer

Hệ thống hỗ trợ nhiều loại thuật toán lập lịch khác nhau, bao gồm:
- Round-Robin
- Multi-Level Queue (MLQ) (có thể bật bằng tùy chọn biên dịch)

### 4. Quản Lý Tiến Trình (Process Management)

- **loader.c/loader.h**: Nạp các tiến trình vào bộ nhớ
- **queue.c/queue.h**: Cấu trúc hàng đợi cho tiến trình

### 5. Syscall

- **syscall.c/syscall.h**: Cài đặt giao diện lời gọi hệ thống
- **syscall.tbl**: Bảng định nghĩa các lời gọi hệ thống
- **sys_killall.c**: Lời gọi hệ thống để dừng tất cả các tiến trình
- **sys_mem.c**: Lời gọi hệ thống liên quan đến quản lý bộ nhớ
- **sys_listsyscall.c**: Lời gọi hệ thống liệt kê tất cả syscall được hỗ trợ

### 6. Thư viện tiêu chuẩn

- **libstd.c**: Thư viện các hàm tiêu chuẩn
- **libmem.c/libmem.h**: Thư viện hỗ trợ quản lý bộ nhớ

## Luồng Hoạt Động

1. **Khởi động hệ thống**:
   - Đọc file cấu hình để lấy thông tin về số lượng CPU, thời gian time slot và danh sách tiến trình
   - Khởi tạo bộ nhớ RAM và SWAP
   - Khởi tạo các CPU ảo dưới dạng các thread

2. **Nạp tiến trình**:
   - Thread loader (`ld_routine`) nạp các tiến trình vào bộ nhớ theo thời gian bắt đầu được chỉ định
   - Mỗi tiến trình được cấp phát không gian bộ nhớ ảo

3. **Lập lịch và chạy tiến trình**:
   - Các CPU ảo (threads trong `cpu_routine`) lấy tiến trình từ hàng đợi sẵn sàng
   - Thực hiện chạy tiến trình trong một time slot
   - Sau khi hết time slot, tiến trình được đưa trở lại hàng đợi (nếu chưa hoàn thành)

4. **Quản lý bộ nhớ**:
   - Tiến trình truy cập bộ nhớ thông qua cơ chế phân trang
   - Khi RAM đầy, dữ liệu có thể được swap ra các khu vực swap

5. **Kết thúc**:
   - Khi tất cả tiến trình đã được nạp và thực thi xong, hệ thống kết thúc

## Mối Liên Kết Giữa Các Module

```
           +-------+
           |  os.c |  (Điều khiển chính)
           +-------+
               |
    +----------+----------+
    |          |          |
+-------+  +-------+  +--------+
| cpu.c |  | mm.c  |  | sched.c|
+-------+  +-------+  +--------+
    |          |          |
    |      +---+---+      |
    |      |       |      |
+-------+ +-----+ +-------+ +---------+
|loader.c| |mem.c| |timer.c| |syscall.c|
+-------+ +-----+ +-------+ +---------+
                                 |
                          +------+------+
                          |      |      |
                  +------------+ | +------------+
                  |sys_killall.c| |sys_listsyscall.c|
                  +------------+ +------------+
```

## Cách Biên Dịch và Chạy

1. **Biên dịch toàn bộ hệ thống**:
   ```
   make
   ```

2. **Chạy hệ thống mô phỏng**:
   ```
   ./os <đường_dẫn_đến_file_cấu_hình>
   ```

3. **Định dạng file cấu hình**:
   ```
   <time_slot> <số_lượng_CPU> <số_lượng_tiến_trình>
   <thời_gian_bắt_đầu_1> <tên_tiến_trình_1> [<độ_ưu_tiên_1>]
   <thời_gian_bắt_đầu_2> <tên_tiến_trình_2> [<độ_ưu_tiên_2>]
   ...
   ```

## Tùy Chọn Biên Dịch

Hệ thống hỗ trợ các tùy chọn biên dịch sau:
- **MM_PAGING**: Bật hỗ trợ phân trang bộ nhớ
- **MLQ_SCHED**: Bật lập lịch đa mức
- **MM_FIXED_MEMSZ**: Sử dụng kích thước bộ nhớ cố định 


Chạy lệnh trên Linux nha mấy cu

Hiện thực hệ thống:
I/Scheduling:

II/ Management Memory:
1/Hiện thực hàm MEMPHY_dump trong mm-memphy.c (in ra thông tin của bộ nhớ(mô phỏng RAM) để sửa lỗi)
2/ hiện thực get_trans_table() trong mem.c (xử lí cụ thể cấp độ dịch trong phân trang 2 cấp của TLB)
3/Làm việc với file mm.c:
   3.1/Hiện thực hàm init_mm (khởi tạo cấu trúc quản lí bộ nhớ cho tiến trình)