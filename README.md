# EDR_Kernel_Agent
This repository is Kernel based Agent program for EDR(Endpoint Detection Response)

## For a Windows
```
  1차 개발완료!
  단, 미니필터 pre 핸들러에서 IRP_MJ_READ는 검토중입니다. (당연히 넣어야 하는 요소이지만, 이벤트가 많아 연구필요)

  -- 개발 수정!
  [미니 필터]
    -> PRE 미니필터 핸들러에서 모니터링 정보를 수집하던 로직을 POST 미니필터로 이전하여 파일 크기를 항상 얻어올 수 있도록 함.
    -> PRE 미니필터는 "차단용" 핸들러로 설계/구현
```
Detail Description here! -> [https://cominam-documents.gitbook.io/cominam-edr/agent/windows-os](https://cominam-documents.gitbook.io/cominam-edr/agent/windows-os)
<br>

## For a Linux

```
  Preparing...
```

<br>
