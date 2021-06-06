#include "../job_manger.h"
#include "unix_api.h"

int main() {
        struct job_t *job1, *job2, *job3;

        init_job();

        job1 = create_job();
        job2 = create_job();
        job3 = create_job();

        add_process(job1, 110, "run job1 110", FORGROUND_RUNNING);
        add_process(job1, 111, "run job1 111", FORGROUND_RUNNING);
        add_process(job1, 112, "run job1 112", FORGROUND_RUNNING);

        add_process(job2, 120, "run job2 120", FORGROUND_RUNNING);
        add_process(job2, 121, "run job2 121", FORGROUND_RUNNING);
        add_process(job2, 122, "run job2 122", FORGROUND_RUNNING);

        add_job(job1, "run job1 110 && run job1 111 && run job1 112");
        add_job(job2, "run job2 120 && run job2 121 && run job2 122");

        del_process(120);
        del_process(121);
        del_process(122);

        add_process(job3, 130, "run job3 130", FORGROUND_RUNNING);
        add_process(job3, 131, "run job3 131", FORGROUND_RUNNING);
        add_process(job3, 132, "run job3 132", FORGROUND_RUNNING);
        add_job(job3, "run job3 130 && run job3 131 && run job3 132");

        list_job();
}
