FROM thingsboard/tb-node:4.2.0

ENV TB_SERVICE_ID=tb-ce-node \
    SPRING_DATASOURCE_URL=jdbc:postgresql://aws-1-us-east-2.pooler.supabase.com:5432/postgres?sslmode=require \
    SPRING_DATASOURCE_USERNAME=postgres.tookehdxqxvercbnfngn \
    SPRING_DATASOURCE_PASSWORD=75OpcUorLhA5yr2z \
    INSTALL_TB=true

EXPOSE 8080

CMD ["./docker-entrypoint.sh"]
