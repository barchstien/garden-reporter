FROM python:3.10-slim

RUN pip install pipenv

ENV PROJECT_DIR /usr/local/garden-collector
WORKDIR ${PROJECT_DIR}
COPY Pipfile Pipfile.lock ${PROJECT_DIR}/
USER garden
RUN pipenv install --system --deploy --ignore-pipfile

RUN mkdir -p ${PROJECT_DIR}/source
COPY source/* ${PROJECT_DIR}/source/

# TODO env

#CMD ["/root/default_script.sh"]

